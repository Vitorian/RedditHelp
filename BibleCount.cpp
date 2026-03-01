#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

// Download text from
// https://raw.githubusercontent.com/mxw/grmr/master/src/finaltests/bible.txt

static std::string slurp(std::istream& in)
{
	// Read entire stream into one contiguous string.
	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}

template <typename ByteSink>
struct BitCache {
	// Packs variable-sized bit chunks into a 64-bit cache and emits bytes to sink.
	explicit BitCache(ByteSink& fcn)
	    : sink(fcn)
	{
		cache = 0;
		counter = 0;
	}

	void commit()
	{
		// Cache is full (64 bits): emit 8 bytes, least-significant byte first.
		for (uint32_t j = 0; j < 8; ++j) {
			sink.push(uint8_t(cache));
			cache >>= 8U;
		}
	}

	void push(uint64_t bits, uint32_t count)
	{
		// Append 'count' bits from 'bits' into cache, committing as needed.
		if (counter + count > 64) {
			cache |= bits << counter;
			commit();
			uint32_t remaining = 64 - counter;
			cache = bits >> remaining;
			counter = count - remaining;
		} else if (counter + count == 64) {
			cache |= bits << counter;
			commit();
			cache = 0;
			counter = 0;
		} else {
			cache |= bits << counter;
			counter += count;
		}
	}

	void flush()
	{
		// Emit any partially filled bytes at end of stream.
		while (counter > 0) {
			sink.push(uint8_t(cache));
			cache >>= 8U;
			if (counter >= 8) {
				counter -= 8;
			} else {
				counter = 0;
			}
		}
		cache = 0;
	}

	uint64_t cache;
	uint32_t counter;
	ByteSink& sink;
};

template <typename Store, uint32_t MAXBITS>
struct VarCodeFilter {
	// Encodes upper/lower case runs into variable-length codewords.
	explicit VarCodeFilter(Store& s)
	    : store(s)
	{
		counter = 0;
	}
	void push(bool isupper)
	{
		// Uppercase emits full-length marker; lowercase extends the run.
		if (isupper) {
			pack();
			store.push(1U << (MAXBITS - 1), MAXBITS);
		} else {
			counter++;
			if (counter == MAXBITS - 1) {
				pack();
			}
		}
	}
	void pack()
	{
		// Flush pending lowercase run as a shorter marker.
		if (counter > 0) {
			uint32_t nbits = MAXBITS - counter;
			store.push(1U << (nbits - 1), nbits);
			counter = 0;
		}
	}
	void flush()
	{
		pack();
		store.flush();
	}
	uint32_t counter;
	Store& store;
};

struct ByteStorage
{
	// Minimal growable byte buffer used by the bit writer.
	ByteStorage()
	{
		count = 0;
		capacity = 4096;
		ptr = static_cast<uint8_t*>(::malloc(capacity));
	}
	ByteStorage(const ByteStorage& rhs)
	{
		*this = rhs;
	}
	ByteStorage& operator=(const ByteStorage& rhs)
	{
		if (this == &rhs) {
			return *this;
		}

		::free(ptr);
		count = rhs.count;
		capacity = rhs.capacity;
		ptr = static_cast<uint8_t*>(::malloc(rhs.capacity));
		std::memcpy(ptr, rhs.ptr, count);
		return *this;
	}
	ByteStorage(const ByteStorage&& rhs) = delete;
	ByteStorage& operator=(const ByteStorage&& rhs) = delete;

	~ByteStorage()
	{
		::free(ptr);
		count = 0;
		capacity = 0;
		ptr = nullptr;
	}
	void push(uint8_t byte)
	{
		// Double capacity on demand.
		if (count == capacity) {
			capacity = 2 * capacity;
			ptr = static_cast<uint8_t*>(::realloc(ptr, capacity));
		}
		ptr[count++] = byte;
	}
	uint32_t size() const
	{
		return count;
	}
	const uint8_t* begin() const
	{
		return ptr;
	}
	const uint8_t* end() const
	{
		return ptr + count;
	}
	uint8_t operator[](uint32_t index) const
	{
		return ptr[index];
	}
	uint8_t& operator[](uint32_t index)
	{
		return ptr[index];
	}
	uint8_t* ptr;
	uint32_t count;
	uint32_t capacity;
};

template <unsigned NBITS>
struct CompressedStream {
	// Pipeline: VarCodeFilter -> BitCache -> ByteStorage.
	CompressedStream()
	    : cache(store)
	    , varicode(cache)
	{
	}
	void push(bool bit)
	{
		varicode.push(bit);
	}
	void flush()
	{
		varicode.flush();
	}
	using Store = ByteStorage;
	using Cache = BitCache<Store>;
	using Encoder = VarCodeFilter<Cache, NBITS>;
	Store store;
	Cache cache;
	Encoder varicode;
};

static std::string to_binary_str(uint8_t v)
{
	// Show byte as bit string (least-significant bit first).
	char str[8];
	for (uint32_t k = 0; k < 8; ++k) {
		str[k] = (((v & 1) == 0) ? '0' : '1');
		v >>= 1;
	}
	return std::string(str, 8);
}

void process_file(std::istream& ifs)
{
	// Build one compressed stream per uppercase-normalized character.
	using Stream = CompressedStream<3>;
	std::array<Stream, 256> bstream;
	std::string text = slurp(ifs);
	uint32_t totalbytes = 0;
	uint32_t origbytes = 0;
	for (char ch : text) {
		unsigned char uch = static_cast<unsigned char>(ch);
		if (std::isprint(uch) != 0) {
			// Normalize by uppercase character, store whether original was uppercase.
			int upch = std::toupper(uch);
			bool isupper = (upch == static_cast<int>(uch));
			bstream[upch].push(isupper);
			origbytes += 1;
		}
	}

	for (uint32_t j = 0; j < 256; ++j) {
		Stream& cs(bstream[j]);
		cs.flush();
		if (cs.store.size() != 0) {
			totalbytes += cs.store.size();
			std::cout << "Array " << j << " "
			          << "[" << char(j) << "] "
			          << cs.store.size() << " bytes " << std::endl;
			for (uint8_t v : cs.store) {
				std::cout << to_binary_str(v) << ' ';
			}
			std::cout << '\n';
		}
	}
	std::cout << "Total original bytes: " << origbytes << "\n";
	std::cout << "Total processed bytes: " << totalbytes << "\n";
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		process_file(std::cin);
	} else {
		std::ifstream ifs(argv[1]);
		if (!ifs.good()) {
			return 1;
		}
		process_file(ifs);
	}
	return 0;
}

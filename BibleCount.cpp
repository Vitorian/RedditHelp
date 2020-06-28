#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

// Download text from
// https://raw.githubusercontent.com/mxw/grmr/master/src/finaltests/bible.txt

static std::string slurp(std::istream& in)
{
	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}

template <typename ByteSink>
struct BitCache {
	explicit BitCache(ByteSink& fcn)
	    : sink(fcn)
	{
		cache = 0;
		counter = 0;
	}

	void commit()
	{
		for (uint32_t j = 0; j < 8; ++j) {
			sink.push(uint8_t(cache));
			cache >>= 8U;
		}
	}

	void push(uint64_t bits, uint32_t count)
	{
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
	explicit VarCodeFilter(Store& s)
	    : store(s)
	{
		counter = 0;
	}
	void push(bool isupper)
	{
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
		count = rhs.count;
		capacity = rhs.capacity;
		ptr = static_cast<uint8_t*>(::malloc(rhs.capacity));
		std::memcpy(ptr, rhs.ptr, capacity);
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
	CompressedStream()
	    : varicode(cache)
	    , cache(store)
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
	Encoder varicode;
	Cache cache;
	Store store;
};

static std::string to_binary_str(uint8_t v)
{
	char str[8];
	for (uint32_t k = 0; k < 8; ++k) {
		str[k] = (((v & 1) == 0) ? '0' : '1');
		v >>= 1;
	}
	return std::string(str, 8);
}

void process_file(std::istream& ifs)
{
	using Stream = CompressedStream<3>;
	std::array<Stream, 256> bstream;
	std::string text = slurp(ifs);
	uint32_t totalbytes = 0;
	uint32_t origbytes = 0;
	for (int ch : text) {
		if (std::isprint(ch) != 0) {
			int upch = std::toupper(ch);
			bool isupper = (upch == ch);
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

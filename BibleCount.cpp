#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <functional>

// Download text from
// https://raw.githubusercontent.com/mxw/grmr/master/src/finaltests/bible.txt

static std::string slurp(std::istream& in) {
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}


template< typename ByteSink >
struct BitCache
{
    BitCache( ByteSink& fcn ) : sink(fcn) {
        cache = 0;
        counter = 0;
    }

    void commit() {
        for ( uint32_t j=0; j<8; ++j ) {
            sink.push( uint8_t(cache) );
            cache >>= 8;
        }
    }

    void push( uint64_t bits, uint32_t count ) {
        if ( counter + count > 64 ) {
            cache |= bits << counter;
            commit();
            uint32_t remaining = 64-counter;
            cache = bits >> remaining;
            counter = count - remaining;
        }
        else if ( counter + count == 64 ) {
            cache |= bits << counter;
            commit();
            cache = 0;
            counter = 0;
        }
        else {
            cache |= bits << counter;
            counter += count;
        }
    }

    void flush() {
        while ( counter > 0 ) {
            sink.push( uint8_t( cache ) );
            cache >>= 8;
            if ( counter>=8 ) counter -= 8;
            else counter = 0;
        }
        cache = 0;
    }

    uint64_t cache;
    uint32_t counter;
    ByteSink& sink;
};

template< typename Store, uint32_t MAXBITS >
struct VarCodeFilter {
    VarCodeFilter( Store& s ) : store(s) {
        counter = 0;
    }
    void push( bool isupper ) {
        if ( isupper ) {
            pack();
            store.push( 1 << (MAXBITS-1), MAXBITS );
        }
        else {
            counter++;
            if ( counter == MAXBITS-1 ) {
                pack();
            }
        }
    }
    void pack() {
        if ( counter>0 ) {
            uint32_t nbits = MAXBITS-counter;
            store.push( 1 << (nbits-1), nbits );
            counter = 0;
        }
    }
    void flush() {
        pack();
        store.flush();
    }
    uint32_t counter;
    Store& store;
};

template < unsigned NBITS >
struct CompressedStream
{
    CompressedStream() : varicode( cache ), cache( *this ) {}
    void process( bool bit ) {
        varicode.push( bit );
    }
    void finish() {
        varicode.flush();
    }
    void push( uint8_t byte ) {
        data.push_back( byte );
    }
    void flush() {
        varicode.flush();
    }
    using Cache = BitCache< CompressedStream<NBITS> >;
    using Encoder = VarCodeFilter< Cache, NBITS >;
    Encoder varicode;
    Cache cache;
    std::vector<uint8_t> data;
};

void process_file( std::istream& ifs )
{
    using Stream = CompressedStream<3>;
    std::array<Stream,256> bstream;
    std::string text = slurp( ifs );
    uint32_t totalbytes = 0;
    uint32_t origbytes = 0;
    for ( int ch : text ) {
        if ( std::isprint( ch ) ) {
            int upch = std::toupper( ch );
            bool isupper = (upch==ch);
            bstream[upch].process( isupper );
            origbytes += 1;
        }
    }
    for ( Stream& cs : bstream ) {
        cs.finish();
    }

    for ( uint32_t j=0; j<256; ++j ) {
        Stream& cs( bstream[j] );
        totalbytes += cs.data.size();
        if ( !cs.data.empty() ) {
            std::cout << "Array " << j << " " << "[" << char(j) << "] "
                      << cs.data.size() << " bytes " << std::endl;
            for ( uint8_t v : cs.data ) {
                for ( uint32_t k=0; k<8; ++k ) {
                    std::cout << ( ((v&1)==0) ? '0' : '1' );
                    v >>= 1;
                }
                std::cout << ' ';
            }
            std::cout << '\n';
        }
    }
    std::cout << "Total original bytes: " << origbytes << "\n";
    std::cout << "Total processed bytes: " << totalbytes << "\n";

}

int main( int argc, char* argv[] )
{
    if ( argc<2 ) {
        process_file( std::cin );
    }
    else {
        std::ifstream ifs( argv[1] );
        if ( !ifs.good() ) return 1;
        process_file( ifs );
    }
    return 0;
}

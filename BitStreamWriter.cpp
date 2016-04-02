#include <stdint.h>
#include <fstream>
#include <iostream>

//
// License - feel free to use and reuse how the hell you wish
// With regards from Vitorian LLC - Scientific consulting
//
// Compile with
//   $ g++ -std=c++11 BitStreamWriter.cpp -ggdb -o bittest
//   $ hexdump -C test.bin
//   00000000  52 65 64 64 69 74 00                              |Reddit.|
//   00000007

template< class Stream >
class BitStreamWriter
{
public:
    BitStreamWriter( Stream& of ) : _of(of), _bits(0), _count(0) {}
    ~BitStreamWriter() { flush(); }
    void pushbit( uint32_t bit ) {
        _bits = (bit<<(sizeof(_bits)-1)) | (_bits>>1);
        if ( ++_count==sizeof(_bits) ) commit();
    }
    void flush() {
        uint32_t leftbits = _count % 8;
        if ( leftbits>0 ) {
            _bits = _bits >> (8-leftbits);
            _count += 8-leftbits;
        }
        commit();
    }
    void commit() {
        if ( _count==0 ) return;
        uint32_t bytes = (_count-1)/8+1;
        uint32_t left = _count % 8;
        if ( bytes>0 ) {
            _of.write( (const char*)&_bits, bytes );
            _count = _count % 8;
            _bits = _bits >> (bytes*8);
        }
    }
private:
    Stream& _of;
    uint64_t _bits;
    uint32_t _count;
};


int main()
{
    std::ofstream ofs( "test.bin" );
    BitStreamWriter<std::ofstream> w( ofs );
    for ( auto ch : "Reddit" ) {
        //std::cout << "New byte:" << std::hex << ch << " " << int(ch) << std::dec << std::endl;
        for ( uint32_t j=0; j<8; ++j ) {
            w.pushbit( ch&1 );
            //std::cout << "Bit:" << (ch&1) << std::endl;
            ch >>= 1;
        }

    }
    w.flush();
    std::cout << std::endl;
    return 0;
}

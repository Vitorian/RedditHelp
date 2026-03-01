#include <stdint.h>
#include <fstream>

#include "BitStreamWriter.h"

//
// License - feel free to use and reuse how the hell you wish
// With regards from Vitorian LLC - Scientific consulting
//
// Compile with
//   $ g++ -std=c++11 BitStreamWriter.cpp -ggdb -o bittest
//   $ hexdump -C test.bin
//   00000000  52 65 64 64 69 74 00                              |Reddit.|
//   00000007

#ifndef BITSTREAMWRITER_NO_MAIN
int main()
{
    std::ofstream ofs( "test.bin" );
    BitStreamWriter<std::ofstream> w( ofs );

    // Writes "Reddit\0" to the output by pushing bytes bit-by-bit (LSB first).
    for ( auto ch : "Reddit" ) {
        for ( uint32_t j=0; j<8; ++j ) {
            w.pushbit( ch&1 );
            ch >>= 1;
        }
    }

    w.flush();
    return 0;
}
#endif

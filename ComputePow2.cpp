#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <cassert>

void printpow2( uint32_t N )
{
    const uint64_t MAXPOW10 = 1000000000ULL;
    const uint32_t NDIG  = 9;
    const uint32_t NBITS = 29; //2**29 = 536870912
    const uint64_t MASK = (1<<NBITS)-1;

    uint32_t nblocks = N/NBITS + 1;
    std::vector<uint32_t> vec(nblocks, 0);
    vec[0] = 1;
    uint32_t usedblocks = 1;
    uint32_t nbitsleft = N;
    while ( nbitsleft>0 ) {
        uint32_t nb = nbitsleft >= NBITS ? NBITS : nbitsleft;
        nbitsleft -= nb;
        uint64_t carry = 0;
        for ( uint32_t k=0; k<usedblocks; ++k ) {
            uint64_t v = (uint64_t(vec[k]) << nb) + carry;
            vec[k] = v % MAXPOW10;
            carry  = v / MAXPOW10;
        }
        if ( carry>0 ) {
            vec[usedblocks] = carry;
            usedblocks++;
        }
    }

    printf( "%d", vec[usedblocks-1] );
    for ( uint32_t k=1; k<usedblocks; ++k ) {
        printf( "%09d", vec[usedblocks-k-1] );
    }
    printf( "\n" );
}

int main()
{
    printpow2( 10000 );
}

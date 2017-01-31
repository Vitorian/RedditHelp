#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>

struct Calc {
    uint32_t maxblocks;
    std::atomic<int32_t> nbits;
    std::vector< std::atomic<uint32_t> > values;
    Calc( uint32_t n ) : values(n) {
        for ( auto& v: values ) { v=0; }
    }
};

const uint64_t MAXPOW10 = 1000000000ULL;
const uint32_t NBITS = 29; //2**29 = 536870912

inline void WAIT() {
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void calcblocks( uint32_t thnum,
                 std::atomic<int32_t>& next,
                 std::atomic<int32_t>& counter,
                 Calc& c )
{
    while ( true ) {
        // The order does not matter
        int32_t nb = c.nbits.fetch_sub( NBITS );
        if ( nb<=0 ) { counter=-2; break; }
        if ( nb>NBITS ) nb = NBITS;

        // each thread does one sweep
        uint64_t carry = 0;
        for ( uint32_t k=0; k<c.maxblocks; k++ ) {
            while ( next==k ) WAIT();
            counter = k;

            uint64_t v = (uint64_t(c.values[k]) << nb) + carry;
            c.values[k] = v % MAXPOW10;
            carry  = v / MAXPOW10;
        }
        // wait until next has launched a new row
        while ( next==-1 ) WAIT();
        counter = -1;
   }
}

void printpow2( uint32_t N, uint32_t nt )
{
    uint32_t nblocks = N/NBITS + 1;
    Calc calc(nblocks);
    calc.values[0] = 1;
    calc.nbits = N;
    calc.maxblocks = nblocks;

    std::vector< std::thread > threads;
    std::vector< std::atomic<int32_t> > flags(nt);
    std::fill( flags.begin(), flags.end(), -1 );

    for ( int32_t j=0; j<nt; ++j ) {
        threads.push_back( std::thread( calcblocks, j,
                                        std::ref(flags[j==0?nt-1:j-1]),
                                        std::ref(flags[j]),
                                        std::ref(calc) ) );
    }
    for ( auto& th: threads ) th.join();

    uint32_t numslots = 0;
    for ( uint32_t k=calc.maxblocks-1; k>0; --k ) {
        if ( (uint32_t)calc.values[k]!=0 ) {
            numslots = k+1;
            break;
        }
    }
    printf( "%d", (uint32_t)calc.values[numslots-1] );
    for ( uint32_t k=1; k<numslots; ++k ) {
        printf( "%09d", (uint32_t)calc.values[numslots-k-1] );
    }
    printf( "\n" );
}

int main()
{
    printpow2( 100000, 4 );
}

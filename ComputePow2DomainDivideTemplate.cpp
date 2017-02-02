#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>

const uint64_t MAXPOW10 = 1000000000ULL;
const uint32_t NBITS = 29; //2**29 = 536870912

inline void WAIT() {
    asm( "pause\n" );
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}

struct Sync {
    std::atomic< uint32_t > nbits;
    std::atomic< uint32_t > carry;
    std::atomic< uint32_t > counter;
    Sync() : nbits(0), carry(0), counter(0) {}
};

template< bool ISHEAD, bool HASNEXT >
void calcblocks( uint32_t bitsleft, uint32_t* values, uint32_t count,
                 Sync& in, Sync& out )
{
    uint32_t numvals = 0;
    for ( uint32_t k=0; k<count; ++k ) {
        if ( values[k]!=0 ) {
            numvals = k+1;
        }
    }

    while ( true ) {
        uint32_t nb;
        uint64_t carry;
        if ( ISHEAD ) {
            if ( bitsleft==0 ) {
                if ( HASNEXT ) { 
                    // post END-OF-JOB and propagate
                    while ( (out.counter & 1)==1 ) WAIT();
                    out.nbits = 0;
                    out.counter++;
                }
                break;
            }
            nb = std::min( NBITS, bitsleft );
            carry = 0;
            bitsleft -= nb;
        }
        else {
            while ( (in.counter & 1) == 0 ) WAIT();
            nb = in.nbits;
            carry = in.carry;
            in.counter++;

            if ( nb==0 ) { // END-OF-JOB
                if ( HASNEXT ) {
                    while ( (out.counter&1)==1 ) WAIT();
                    out.nbits = 0;
                    out.counter++;
                }
                break;
            }
        }

        if ( (numvals==0) and (carry==0) ) continue;

        // each thread does one sweep
        for ( uint32_t k=0; k<numvals; k++ ) {
            uint64_t v = (uint64_t(values[k]) << nb) + carry;
            values[k] = v % MAXPOW10;
            carry  = v / MAXPOW10;
        }

        if ( numvals<count ) {
            if ( carry>0 ) {
                values[numvals] = carry;
                numvals++;
            }
        } else {
            if ( HASNEXT ) {
                while ( (out.counter & 1) == 1 ) WAIT();
                out.nbits = nb;
                out.carry = carry;
                out.counter++;
            }
        }
   }
}


void printpow2( uint32_t N, uint32_t nt )
{
    uint32_t nblocks = N/NBITS + 1;
    uint32_t slice = nblocks/nt+1;
    if ( slice<4 ) slice = 4;
    uint32_t maxthreads = nblocks/slice + 1;
    if ( nt>maxthreads ) nt = maxthreads;

    std::vector< std::thread > threads;
    std::vector< Sync > sync(nt+1);
    std::vector< uint32_t > values(nblocks,0);
    values[0] = 1;

    for ( int32_t j=0; j<nt; ++j ) {
        uint32_t jmin = slice*j;
        uint32_t jcount = std::min( slice, nblocks-jmin );
        if ( j==0 ) {
            if ( nt==1 ) {
                // sole blocks
                threads.push_back( std::thread( calcblocks<true,false>,
                                                N, &values[jmin], jcount,
                                                std::ref(sync[j]),
                                                std::ref(sync[j+1]) ));
            } else {
                // first blocks
                threads.push_back( std::thread( calcblocks<true,true>,
                                                N, &values[jmin], jcount,
                                                std::ref(sync[j]),
                                                std::ref(sync[j+1]) ));
            }
        }
        else if ( j==nt-1 ) {
            // end tiles
            threads.push_back( std::thread( calcblocks<false,false>,
                                            N, &values[jmin], jcount,
                                            std::ref(sync[j]),
                                            std::ref(sync[j+1]) ));
        }
        else {
            // middle tiles
            threads.push_back( std::thread( calcblocks<false,true>,
                                            N, &values[jmin], jcount,
                                            std::ref(sync[j]),
                                            std::ref(sync[j+1]) ));
        }
    }
    for ( auto& th: threads ) th.join();

    uint32_t numslots = 1;
    for ( uint32_t k=nblocks-1; k>0; --k ) {
        if ( (uint32_t)values[k]!=0 ) {
            numslots = k+1;
            break;
        }
    }

    printf( "%d", (uint32_t)values[numslots-1] );
    for ( uint32_t k=1; k<numslots; ++k ) {
        printf( "%09d", (uint32_t)values[numslots-k-1] );
    }
    printf( "\n" );
}

int main()
{
    printpow2( 2000000, 4 );
}

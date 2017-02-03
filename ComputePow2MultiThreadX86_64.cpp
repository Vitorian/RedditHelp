#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>

const uint64_t MAXPOW10 = 10000000000000000000ULL;
const uint32_t NBITS = 63; //2**63 = 9 223 372 036 854 775 808

inline void WAIT() {
    asm( "pause\n" );
    //std::this_thread::sleep_for(std::chrono::microseconds(1));
}

static inline void div64( uint64_t low, uint64_t high, uint64_t divisor,
                          uint64_t& quotient, uint64_t& remainder )
{
    asm("divq %2" : "+a"(low),"+d"(high) : "rm"(divisor) );
    quotient = high;
    remainder = low;
}

static inline void add64( uint64_t low, uint64_t high,
                          uint64_t& r,  uint64_t& carry )
{
    asm ("addq %2,%0; adcq %3,%1"
         : "+r"(r),"+d"(high)
         : "a"(low),"g"(carry)
         : "cc");
    carry = high;
}

struct Sync {
    std::atomic< uint32_t > nbits;
    std::atomic< uint64_t > carry;
    std::atomic< uint32_t > counter;
    Sync() : nbits(0), carry(0), counter(0) {}
};

void calcblocks( uint32_t thnum, uint32_t numthreads,
                 uint32_t bitsleft, uint64_t* values, uint32_t count,
                 Sync& in, Sync& out )
{
    bool ISHEAD = (thnum==0);
    bool HASNEXT = (thnum<numthreads-1);

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
            uint64_t lo = (values[k] << nb);
            uint64_t hi = values[k] >> (64-nb);
            add64( carry, 0, lo, hi );
            div64( lo, hi, MAXPOW10, values[k], carry );
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
    std::vector< uint64_t > values(nblocks,0);
    values[0] = 1;

    for ( int32_t j=0; j<nt; ++j ) {
        uint32_t jmin = slice*j;
        uint32_t jcount = std::min( slice, nblocks-jmin );
        threads.push_back( std::thread( calcblocks,
                                        j, nt, N,
                                        &values[jmin], jcount,
                                        std::ref(sync[j]),
                                        std::ref(sync[j+1]) ));
    }
    for ( auto& th: threads ) th.join();

    uint32_t numslots = 1;
    for ( uint32_t k=nblocks-1; k>0; --k ) {
        if ( (uint32_t)values[k]!=0 ) {
            numslots = k+1;
            break;
        }
    }

    printf( "%lu", (uint64_t)values[numslots-1] );
    for ( uint32_t k=1; k<numslots; ++k ) {
        printf( "%019lu", (uint64_t)values[numslots-k-1] );
    }
    printf( "\n" );
}

int main( int argc, char* argv[] )
{
    if ( argc<3 ) {
        printf( "Usage: %s <nbits> <nthreads>\n", argv[0] );
        return 0;
    }
    uint32_t nbits = ::atol( argv[1] );
    uint32_t nthreads = ::atol( argv[2] );
    printpow2( nbits, nthreads );
}

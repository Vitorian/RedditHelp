
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

template< class Fn >
void genPermutations( const uint32_t N, const uint32_t R, Fn&& fn )
{
    if ( (N<1) or (R>N) ) return;

    std::vector<uint32_t> idx(N);
    for ( uint32_t j=0; j<N; ++j ) idx[j] = j;

    std::vector<uint32_t> cyc(R);
    for ( uint32_t j=0; j<R; ++j ) cyc[j] = N-j;

    std::vector<uint32_t> cur(R);
    std::copy( &idx[0], &idx[R], &cur[0] );
    fn( 0, cur );

    uint32_t count = 0;
    bool gotit;
    do {
        gotit = false;
        uint32_t i = R;
        while ( i>0 ) {
            --i;
            cyc[i]--;
            if ( cyc[i] == 0 ) {
                uint32_t first = idx[i];
                for ( uint32_t j=i; j<N-1; ++j ) idx[j] = idx[j+1];
                idx[N-1] = first;
                cyc[i] = N-i;
            }
            else {
                uint32_t j = cyc[i];
                std::swap( idx[i], idx[N-j] );
                std::copy( &idx[0], &idx[R], &cur[0] );
                fn( ++count, cur );
                gotit = true;
                break;
            }
        }
    } while( gotit );
}

struct Ring {
    int32_t elem[4];
};

static inline double calc_ring_power( int32_t pw0, int32_t pw1, int32_t pw2, int32_t pw3, int32_t pw4 )
{
    double s1 = 0 + std::pow(pw0-2,2);
    double s2 = (s1-30) + 5*std::abs(pw1-5);
    double s3 = -s2 + pw2%3;
    double s4 = std::floor(std::fabs(s3)/2) + std::pow((pw3-7),2);
    double s5 = (100-s4) + (10-pw4);
    return s5;
}

void process( std::istream& ifs ) {
    std::string line;
    uint32_t numrings;
    std::getline( ifs, line );
    std::istringstream iheader( line );
    iheader >> numrings;

    std::vector< Ring > rings(numrings);
    for ( uint32_t j=0; j<numrings; ++j ) {
        std::getline( ifs, line );
        std::istringstream iis( line );
        Ring& rng( rings[j] );
        iis >> rng.elem[0] >> rng.elem[1] >> rng.elem[2] >> rng.elem[3];
    }

    double bestpw[4] = {0,0,0,0};
    genPermutations( numrings, 5,
                     [&bestpw,&rings]( uint32_t pm_count,
                        const std::vector<uint32_t>& perm ) {
           double pw[4];
           for ( uint32_t k = 0; k<4; ++k ) {
               pw[k] = calc_ring_power(rings[ perm[0] ].elem[k],
                                       rings[ perm[1] ].elem[k],
                                       rings[ perm[2] ].elem[k],
                                       rings[ perm[3] ].elem[k],
                                       rings[ perm[4] ].elem[k] );
               if ( pw[k]<80 ) return;
           }

           printf( ">> Perm %d:  %d %d %d %d %d  ",
                   pm_count, perm[0], perm[1], perm[2], perm[3], perm[4]  );
           for ( uint32_t k=0; k<4; ++k ) {
               if ( pw[k]>bestpw[k] ) {
                   bestpw[k] = pw[k];
                   printf( " [%2.0f] ", pw[k] );
               } else {
                   printf( "  %2.0f  ", pw[k] );
               }
           }
           printf( "\n" );
    });

    for ( uint32_t k=0; k<4; ++k ) {
        printf( "%.0f\n", bestpw[k] );
    }
}

int main( int argc, char* argv[] )
{

    if ( argc>1 ) {
        for ( uint32_t j=1; j<argc; ++j ) {
            std::ifstream ifs( argv[j] );
            process( ifs );
        }
    }
    else {
        process( std::cin );
    }
}

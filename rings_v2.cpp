
#include <cmath>     // for std::abs, std::pow, std::fabs
#include <cstdint>   // I dont use int, long, long long etc b/c they are ambiguous
#include <iostream>  // for std::cin
#include <fstream>   // for std::ifstream
#include <sstream>   // for std::istringstream
#include <algorithm> // for std::swap, std::copy


// Represents a ring
struct Ring {
    int32_t elem[4];
};

// This should in fact be inside a class but this is just an example
std::vector<Ring> rings;
double bestpw[4];

// Computes the stacked power of a set of rings - for a single element
static inline double calc_ring_power( int32_t pw0, int32_t pw1, int32_t pw2, int32_t pw3, int32_t pw4 )
{
    double s1 = 0 + std::pow(pw0-2,2);
    double s2 = (s1-30) + 5*std::abs(pw1-5);
    double s3 = -s2 + pw2%3;
    double s4 = std::floor(std::fabs(s3)/2) + std::pow((pw3-7),2);
    double s5 = (100-s4) + (10-pw4);
    return s5;
}

// Receives the callback from gen_permutations() for each new permutation
void new_permutation( uint32_t pm_count, std::vector<uint32_t>& perm )
{
    double pw[4];
    for ( uint32_t k = 0; k<4; ++k ) {
        pw[k] = calc_ring_power(rings[ perm[0] ].elem[k],
                                rings[ perm[1] ].elem[k],
                                rings[ perm[2] ].elem[k],
                                rings[ perm[3] ].elem[k],
                                rings[ perm[4] ].elem[k] );

        // No good if the power is less than the minimum
        if ( pw[k]<80 ) return;
    }

    // Print the ring IDs
    printf( ">> Perm %d:  %d %d %d %d %d  ",
            pm_count, perm[0], perm[1], perm[2], perm[3], perm[4]  );

    // For each element, compare the current solution to the best solution so far
    for ( uint32_t k=0; k<4; ++k ) {
        if ( pw[k]>bestpw[k] ) {
            bestpw[k] = pw[k];
            // print in a highlighted way for fanciness - these are just comments
            printf( " [%2.0f] ", pw[k] );
        } else {
            printf( "  %2.0f  ", pw[k] );
        }
    }
    printf( "\n" );
}


// Generate all permutations - this should be N! permutations
// This is VERY wasteful if N is much greater than R
void gen_permutations_wasteful( const uint32_t N, const uint32_t R )
{
    // Create an index of rings and initialize with 1,2,3,4,5...,N
    std::vector<uint32_t> idx(N);
    for ( uint32_t j=0; j<N; ++j ) idx[j] = j;

    // output the first trivial solution
    std::vector<uint32_t> cur(R);
    std::copy( &idx[0], &idx[R], &cur[0] );
    new_permutation( 0, cur );

    while ( std::next_permutation( &idx[0], &idx[N] ) ) {
        std::copy( &idx[0], &idx[R], &cur[0] );
        new_permutation( 0, cur );
    }
}

// Generate all permutations (no order) - this should be N!/(N-R)! permutations
// Taken from https://docs.python.org/2/library/itertools.html#itertools.permutations
void gen_permutations( const uint32_t N, const uint32_t R )
{
    // sanity test
    if ( (N<1) or (R>N) ) return;

    // Create an index of rings and initialize with 1,2,3,4,5...,N
    std::vector<uint32_t> idx(N);
    for ( uint32_t j=0; j<N; ++j ) idx[j] = j;

    // Create a cyclic counter - part of the algorithm
    std::vector<uint32_t> cyc(R);
    for ( uint32_t j=0; j<R; ++j ) cyc[j] = N-j;

    std::vector<uint32_t> cur(R);
    std::copy( &idx[0], &idx[R], &cur[0] );

    // output the first trivial solution
    new_permutation( 0, cur );

    // Counts the number of permutations generated so far
    uint32_t count = 0;

    // We will stop when we could not create anymore permutations
    bool gotit;

    do {
        // assume the worst
        gotit = false;

        // Initialize index
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

                // Swap two elements in the index array
                std::swap( idx[i], idx[N-j] );

                // copy the first R elements from the index array to the actual solution array
                std::copy( &idx[0], &idx[R], &cur[0] );
                new_permutation( ++count, cur );
                gotit = true;
                break;
            }
        }
    } while( gotit );
}

// Processes a file (or stdin)
void process( std::istream& ifs ) {
    std::string line;
    uint32_t numrings;

    // first line has to be the number of rings
    std::getline( ifs, line );
    std::istringstream iheader( line );
    iheader >> numrings;

    // Initialize the best solution with zeros
    for ( uint32_t j=0; j<4; ++j ) bestpw[j] = 0;

    // Resize the vector of rings b/c I hate to be doing push_back()
    // If you know the size up front just resize and use it
    // It avoids calling malloc() internally
    rings.resize( numrings );

    // Loop for every ring
    for ( uint32_t j=0; j<numrings; ++j ) {

        // Read line
        std::getline( ifs, line );

        // We use a second istream to parse the line
        // This is slower but much more convenient
        std::istringstream iis( line );
        iis >> rings[j].elem[0]
            >> rings[j].elem[1]
            >> rings[j].elem[2]
            >> rings[j].elem[3];
    }

    // Now that we read all the rings, proceed to create all
    // possible permutations and compute the stacked power of
    // each of them, keeping the element's best
    gen_permutations_wasteful( numrings, 5 );

    // Print the solution
    for ( uint32_t k=0; k<4; ++k ) {
        printf( "%.0f\n", bestpw[k] );
    }
    // say yay
}

int main( int argc, char* argv[] )
{
    if ( argc>1 ) {
        // If we have command line arguments, they should be file names
        // We process them one by one
        // Notice that argv[0] should be the program name (rings)
        for ( uint32_t j=1; j<argc; ++j ) {
            // Open the file
            std::ifstream ifs( argv[j] );
            process( ifs );
        }
    }
    else {
        // Otherwise we expect the data to be piped into stdin
        // as in `rings < rings.txt`
        process( std::cin );
    }
}


#include <stdlib.h> // for srand() and rand()
#include <iostream> // for std::cout
#include <time.h> // for time()
#include <math.h> // for sqrt()


// Post help /u/Josh18293
// https://github.com/HFTrader/RedditHelp/blob/master/SampleStdDeviation.cpp

// Compile with
//   g++ SampleStdDeviation.cpp -o SampleStdDeviation

int main() {
    // Seed the random number system with the current epoch in seconds
    srand( time(NULL) );

    // Generate numbers
    const unsigned N = 20;
    unsigned data[N];
    for ( unsigned j=0; j<N; ++j ) {
        data[j] = rand() % 10;
        std::cout << "Value[" << j << "] = " << data[j] << std::endl;
    }

    // Reduce
    unsigned sum = 0;
    unsigned sum2 = 0;
    for ( unsigned j=0; j<N; ++j ) {
        sum += data[j];
        sum2 += data[j]*data[j];
    }

    // Compute basic stats
    double average = double(sum)/N;
    double variance = double(sum2 - N*average*average)/(N-1);
    double stdev = sqrt( variance );

    // Print
    std::cout << "Average:" << average << std::endl;
    std::cout << "Standard Deviation (corrected):" << stdev << std::endl;
    return 0;
}

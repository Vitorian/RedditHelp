// calculator.cpp — Command-line expression evaluator with benchmarking
//
// Parses each command-line argument as an arithmetic expression, evaluates it,
// and reports the result along with average parse+evaluate time over many
// iterations. Uses RDTSC on x86 for cycle-accurate timing, falling back to
// std::chrono on other architectures.
//
// Usage: calc <expression> [expression2] ...
// Example: calc "2+3*4" "(2+3)*4"

#include "Calculator.h"
#include "Node.h"
#include "Pointer.h"

#include <cstdint>
#include <cstdio>
#include <exception>
#include <string>

// Optimization barrier — prevents the compiler from discarding the result
// of a computation in a benchmarking loop. Uses an inline asm statement
// that declares the value as an input operand, forcing the compiler to
// materialize it. Same technique used by Google Benchmark.
template <typename Tp>
static inline void DoNotOptimize(Tp const& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

using namespace Interpreter;

// Platform-specific high-resolution timer selection.
// On x86/x64: use RDTSC for cycle-accurate measurement.
// On other architectures: fall back to std::chrono nanoseconds.
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define HAS_RDTSC 1
#endif

#ifdef HAS_RDTSC
uint64_t now() {
    return __builtin_ia32_rdtsc();
}
static constexpr const char* time_unit = "cycles";
#else
#include <chrono>
uint64_t now() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
static constexpr const char* time_unit = "ns";
#endif

// Number of iterations for the benchmark loop.
static constexpr int BENCH_ITERATIONS = 10000;

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            printf("Usage: calc <expression>\n");
            return 0;
        }

        Calculator calc;

        // Process each command-line argument as an independent expression.
        for (int j = 1; j < argc; ++j) {
            std::string cmd = argv[j];
            printf("Solving %s\n", cmd.c_str());

            double value;

            // Benchmark loop: parse and evaluate the expression BENCH_ITERATIONS
            // times to get a stable average timing measurement.
            uint64_t start = now();
            for (int k = 0; k < BENCH_ITERATIONS; ++k) {
                Pointer<Node> ast = calc.parse(cmd);

                if (!ast) {
                    printf("Error: failed to parse expression '%s'\n", cmd.c_str());
                    return 1;
                }

                // Evaluate the AST to produce the numeric result.
                value = ast->calc();

                // Prevent the compiler from optimizing away the computation.
                DoNotOptimize(value);
            }

            // Report results: the final computed value and average time per iteration.
            uint64_t stop = now();
            double elapsed = static_cast<double>(stop - start) / double(BENCH_ITERATIONS);
            printf("Result: %f Avg:%.1f %s\n", value, elapsed, time_unit);
        }
    } catch (const std::exception& e) {
        (void)fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
}

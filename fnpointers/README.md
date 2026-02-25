# Function Pointer Dispatch Benchmark

Benchmarks comparing different function dispatch mechanisms in C++: direct calls,
function pointers, switch statements, and virtual functions. The goal is to measure
the overhead of indirect dispatch and branch misprediction when calling one of
several functions based on a runtime value.

## Benchmarks

| Benchmark | Description |
|---|---|
| `BM_Baseline` | Direct interleaved calls to `func1`/`func2`/`func3` in a fixed pattern (no indirection) |
| `BM_Switch` | Switch on `counter % 3` each iteration to select the function |
| `BM_FnPointerVector` | Pre-built `std::vector` of function pointers, iterated and called |
| `BM_FnPointerArray` | Pre-built `std::array` of function pointers, iterated and called |
| `BM_SwitchVector` | Pre-built `std::vector` of choice indices, dispatched via switch |
| `BM_SwitchArray` | Pre-built `std::array` of choice indices, dispatched via switch |
| `BM_Virtual` | `std::array` of base-class pointers (3 derived types), virtual dispatch |
| `BM_Virtual2` | Same as `BM_Virtual` but with 6 derived types (deeper vtable polymorphism) |

## Files

| File | Description |
|---|---|
| `func.h` | Abstract base class `Func` and free function declarations |
| `func.cpp` | Function implementations, derived classes (`Func1`-`Func6`), and factory functions `genFunc`/`genFunc2` |
| `bm_fnpointer.cpp` | Benchmark definitions and `BENCHMARK_MAIN()` entry point |
| `CMakeLists.txt` | Build configuration (requires Google Benchmark) |

## Build

### Host (requires libbenchmark-dev)

```bash
sudo apt install libbenchmark-dev
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE_INIT='-march=native -mtune=native' \
      -B build
cmake --build build -j
./build/bm_fnpointer
```

### Container (Podman)

```bash
# Build the container image (one-time)
podman build -t ubuntu-bench:24.04 ~/git/tests/containers/ubuntu24.04/

# Configure and build
podman run --rm -v $PWD:/src:Z ubuntu-bench:24.04 \
    cmake -DCMAKE_BUILD_TYPE=Release -B build
podman run --rm -v $PWD:/src:Z ubuntu-bench:24.04 \
    cmake --build build -j

# Run the benchmark
podman run --rm -v $PWD:/src:Z ubuntu-bench:24.04 ./build/bm_fnpointer
```

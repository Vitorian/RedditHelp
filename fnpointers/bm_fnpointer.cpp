

#include <benchmark/benchmark.h>
#include <array>
#include <vector>
#include <iostream>

#include "func.h"

using FnPointer = void (*)(int);

#define NUMLOOPS 1000000

static void BM_FnPointerVector(benchmark::State& state) {
    std::vector<FnPointer> fnvec(NUMLOOPS);
    int counter = 0;
    for (FnPointer& fn : fnvec) {
        switch (counter++ % 3) {
            case 0: fn = func1; break;
            case 1: fn = func2; break;
            case 2: fn = func3; break;
        }
    }
    for (auto _ : state) {
        int counter = 0;
        for (const FnPointer& fn : fnvec) {
            fn(counter++);
        }
    }
}

static void BM_FnPointerArray(benchmark::State& state) {
    std::array<FnPointer, NUMLOOPS> fnvec;
    int counter = 0;
    for (FnPointer& fn : fnvec) {
        switch (counter++ % 3) {
            case 0: fn = func1; break;
            case 1: fn = func2; break;
            case 2: fn = func3; break;
        }
    }
    for (auto _ : state) {
        int counter = 0;
        for (const FnPointer& fn : fnvec) {
            fn(counter++);
        }
    }
}

static void BM_Baseline(benchmark::State& state) {
    for (auto _ : state) {
        for (int counter = 0; counter < NUMLOOPS; counter += 3) {
            func1(counter);
            func2(counter + 1);
            func3(counter + 2);
        }
    }
}

static void BM_Switch(benchmark::State& state) {
    for (auto _ : state) {
        for (int counter = 0; counter < NUMLOOPS; ++counter) {
            switch (counter % 3) {
                case 0: func1(counter); break;
                case 1: func2(counter); break;
                case 2: func3(counter); break;
            }
        }
    }
}

static void BM_SwitchVector(benchmark::State& state) {
    std::vector<int> choices(NUMLOOPS);
    for (int counter = 0; counter < NUMLOOPS; ++counter) {
        choices[counter] = counter % 3;
    }
    for (auto _ : state) {
        for (int choice : choices) {
            switch (choice) {
                case 0: func1(choice); break;
                case 1: func2(choice); break;
                case 2: func3(choice); break;
            }
        }
    }
}

static void BM_SwitchArray(benchmark::State& state) {
    std::array<int, NUMLOOPS> choices;
    for (int counter = 0; counter < NUMLOOPS; ++counter) {
        choices[counter] = counter % 3;
    }
    for (auto _ : state) {
        for (int choice : choices) {
            switch (choice) {
                case 0: func1(choice); break;
                case 1: func2(choice); break;
                case 2: func3(choice); break;
            }
        }
    }
}

static void BM_Virtual(benchmark::State& state) {
    std::array<Func*, NUMLOOPS> fnvec;
    int counter = 0;
    for (Func*& fn : fnvec) {
        fn = genFunc(counter++);
    }
    for (auto _ : state) {
        int counter = 0;
        for (Func* fn : fnvec) {
            fn->func(counter++);
        }
    }
}

static void BM_Virtual2(benchmark::State& state) {
    std::array<Func*, NUMLOOPS> fnvec;
    int counter = 0;
    for (Func*& fn : fnvec) {
        fn = genFunc2(counter++);
    }
    for (auto _ : state) {
        int counter = 0;
        for (Func* fn : fnvec) {
            fn->func(counter++);
        }
    }
}

BENCHMARK(BM_Baseline);
BENCHMARK(BM_Switch);
BENCHMARK(BM_FnPointerVector);
BENCHMARK(BM_FnPointerArray);
BENCHMARK(BM_SwitchVector);
BENCHMARK(BM_SwitchArray);
BENCHMARK(BM_Virtual);
BENCHMARK(BM_Virtual2);
BENCHMARK_MAIN();

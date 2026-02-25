// FunctionOps.h — Type-erased function call dispatch via template expansion
//
// Implements a mechanism to call C functions with a runtime-determined number
// of double arguments. The core idea: store all function pointers as FnPtr
// (a common function pointer type), then reinterpret_cast to the correct
// signature at call time using an index_sequence to unpack the argument array.
//
// The switch-based dispatch in callfn() selects the correct template
// instantiation at runtime, supporting 0 to 10 arguments.

#pragma once

#include <cstddef>
#include <limits>
#include <utility>

namespace Interpreter {

// Standards-compliant function pointer storage type.
// Casting between function pointer types is well-defined per [expr.reinterpret.cast]/6,
// unlike void* which is not guaranteed to hold function pointers.
using FnPtr = double(*)();

// Helper alias: maps any size_t index to double, used to expand parameter packs.
template <size_t>
using double_t = double;

// Unpacks the argument array into a function call with the correct number of
// parameters. The index_sequence<I...> provides compile-time indices to
// subscript into the values array, producing: fn(values[0], values[1], ...).
template <std::size_t... I>
double callfn_impl(FnPtr fn, const double* values, std::index_sequence<I...>) {
    using CallType = double (*)(double_t<I>...);
    return reinterpret_cast<CallType>(fn)(values[I]...);
}

// Calls fn with exactly N arguments from the values array.
// Generates the index_sequence<0, 1, ..., N-1> and delegates to callfn_impl.
template <std::size_t N>
double callfn(FnPtr fn, const double* values) {
    using Indices = std::make_index_sequence<N>;
    return callfn_impl(fn, values, Indices{});
}

// Runtime dispatch: selects the correct callfn<N> instantiation based on the
// argument count. Returns NaN for unsupported sizes (>10).
inline double callfn(FnPtr fn, const double* args, size_t size) {
    switch (size) {
        case 0: return callfn<0>(fn, args);
        case 1: return callfn<1>(fn, args);
        case 2: return callfn<2>(fn, args);
        case 3: return callfn<3>(fn, args);
        case 4: return callfn<4>(fn, args);
        case 5: return callfn<5>(fn, args);
        case 6: return callfn<6>(fn, args);
        case 7: return callfn<7>(fn, args);
        case 8: return callfn<8>(fn, args);
        case 9: return callfn<9>(fn, args);
        case 10: return callfn<10>(fn, args);
        default: return std::numeric_limits<double>::quiet_NaN();
    }
}

}  // namespace Interpreter

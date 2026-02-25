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

#include <array>
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
double callfn_impl(FnPtr fn, const double* values, std::index_sequence<I...> /*indices*/) {
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

// Maximum number of function arguments supported by the dispatch table.
static constexpr size_t MAX_FN_ARGS = 32;

// Dispatch function type: calls fn with N pre-determined arguments from an array.
using DispatchFn = double(*)(FnPtr, const double*);

// Builds a compile-time dispatch table: table[N] = &callfn<N> for N in 0..MAX.
// The index_sequence expands into an array initializer of function pointers.
template <std::size_t... I>
constexpr std::array<DispatchFn, sizeof...(I)> make_dispatch_table(std::index_sequence<I...> /*indices*/) {
    return {{ &callfn<I>... }};
}

static constexpr auto dispatch_table = make_dispatch_table(std::make_index_sequence<MAX_FN_ARGS + 1>{});

// Runtime dispatch: indexes into the compile-time table to select the correct
// callfn<N> instantiation. Returns NaN for unsupported sizes.
inline double callfn(FnPtr fn, const double* args, size_t size) {
    if (size > MAX_FN_ARGS) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return dispatch_table[size](fn, args);
}

}  // namespace Interpreter

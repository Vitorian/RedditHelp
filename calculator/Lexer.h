// Lexer.h — Tokenizer and low-level parsing primitives
//
// Despite its name, this is more of a scannerless parser foundation than a
// traditional lexer. It operates directly on characters rather than producing
// a separate token stream. The Calculator inherits from Lexer to get these
// primitives.
//
// Key design patterns:
//   - Backtracking via save/commit/restore on an explicit iterator stack.
//   - StackSaver (RAII guard) ensures the iterator is restored on parse
//     failure and committed only on success, preventing position leaks.
//   - Predicate-driven scanning: test() and skip() accept callable objects
//     (see Predicates.h) that classify characters.
//   - All parse methods return std::optional — empty means "no match" and
//     the caller can try an alternative production.

#pragma once

#include "Predicates.h"
#include "TreeNodes.h"

#include <cmath>
#include <cstddef>
#include <optional>
#include <stack>
#include <string_view>

namespace Interpreter {

// Lexer — character-level parsing engine with backtracking support.
// Maintains a current position (iterator) into a string_view and a stack
// of saved positions for speculative parsing.
struct Lexer {
    using sviterator = std::string_view::iterator;

    // Decimal number base used in digit parsing.
    static constexpr size_t kDecimalBase = 10;

    Lexer() {
    }

    // Resets the input and positions the iterator at the beginning.
    void reset(std::string_view str) {
        code = str;
        it = code.begin();
    }

    std::string_view code;  // the full input expression
    sviterator it;          // current scan position

private:
    // Iterator stack for backtracking. Private to enforce use of StackSaver
    // in all public parsing code.
    std::stack<sviterator> stack;

    // Pushes the current position onto the stack and returns it.
    sviterator save() {
        stack.push(it);
        return it;
    }

    // Pops the saved position without restoring it, effectively committing
    // all characters consumed since the matching save().
    sviterator commit() {
        sviterator tmp = stack.top();
        stack.pop();
        return tmp;
    }

    // Pops the saved position and restores it as current, discarding
    // all characters consumed since save(). Returns the old (discarded) position.
    sviterator restore() {
        sviterator tmp = it;
        it = stack.top();
        stack.pop();
        return tmp;
    }

public:
    friend class StackSaver;

    // StackSaver — RAII guard for speculative parsing.
    // On construction, saves the current position. If commit() is never called,
    // the destructor automatically restores the position (parse failure path).
    // If commit() is called, the destructor commits (parse success path).
    struct StackSaver {
        StackSaver(Lexer* lexer) : _lexer(lexer) {
            _lexer->save();
            _committed = false;
        }
        void commit() {
            _committed = true;
        }
        ~StackSaver() {
            if (_committed) {
                _lexer->commit();
            } else {
                _lexer->restore();
            }
        }
        bool _committed;
        Lexer* _lexer;
    };

    // Tests the current character against a predicate. If it matches,
    // advances the iterator and returns the character. Otherwise returns empty.
    template <typename Fn>
    std::optional<char> test(Fn&& pred) {
        if (it != code.end()) {
            if (pred(*it)) {
                return *it++;
            }
        }
        return {};
    };

    // Consumes characters while the predicate holds, returning the matched
    // span as a string_view. Returns empty if no characters matched.
    template <typename Fn>
    std::optional<std::string_view> skip(Fn&& pred) {
        sviterator start = it;
        for (; it != code.end(); ++it) {
            if (!pred(*it)) {
                break;
            }
        }
        if (it == start) {
            return {};
        }
        // Construct a string_view from the matched range using pointer arithmetic.
        return std::string_view{&*start, size_t(it - start)};
    };

    // Converts a digit-only string_view to size_t without heap allocation.
    // Assumes all characters are valid digits (caller must guarantee this).
    static size_t svtoul(std::string_view str) {
        size_t ival = 0;
        for (char chr : str) {
            ival = kDecimalBase * ival + (chr - '0');
        }
        return ival;
    }

    // Parses an unsigned integer by consuming consecutive digits.
    std::optional<size_t> parseuint() {
        if (auto digits = skip(isdigit())) {
            return {svtoul(digits.value())};
        }
        return {};
    }

    // Consumes and discards any leading whitespace.
    std::optional<std::string_view> skipws() {
        return skip(isspace());
    }

    // Multiplies val by 10^iexp using integer arithmetic (no floating point).
    static size_t ipow10(size_t val, size_t iexp) {
        for (; iexp > 0; --iexp) {
            val = val * kDecimalBase;
        }
        return val;
    }

    // Parses a floating-point number: [+-]? digits ['.' digits]?
    // Uses raw save/commit/restore instead of StackSaver because it needs
    // to commit at two different points (integer-only vs decimal paths).
    std::optional<double> parsedouble() {
        save();
        bool neg = false;

        // Optional leading sign.
        if (auto sgn = test(isany("+-"))) {
            neg = sgn.value() == '-';
        }

        if (auto sint = skip(isdigit())) {
            size_t ival = svtoul(sint.value());  // integer part

            // Try to consume a decimal point followed by fractional digits.
            if (test(ischar('.'))) {
                if (auto sfrac = skip(isdigit())) {
                    size_t fval = svtoul(sfrac.value());  // fractional digits as int
                    size_t len = sfrac.value().size();    // number of fractional digits
                    ival = ipow10(ival, len);             // scale integer part up
                    double dval =
                        static_cast<double>(ival + fval) * pow(kDecimalBase, -static_cast<double>(len));  // combine and scale back down
                    commit();
                    return neg ? -dval : dval;
                }
            }

            // No decimal point — return the integer part as double.
            double dval = static_cast<double>(ival);
            commit();
            return neg ? -dval : dval;
        }

        // No digits found at all — restore position and signal failure.
        restore();
        return {};
    }

    // Tries to consume an arithmetic operator (+, -, *, /) and returns the
    // corresponding BinaryOp::Operation enum value.
    std::optional<BinaryOp::Operation> arithop() {
        if (auto sym = test(isany("-+*/"))) {
            switch (sym.value()) {
                case '+': return BinaryOp::Operation::Addition;
                case '-': return BinaryOp::Operation::Subtraction;
                case '*': return BinaryOp::Operation::Multiplication;
                case '/': return BinaryOp::Operation::Division;
                default: break;
            }
        }
        return {};
    }
};

}  // namespace Interpreter

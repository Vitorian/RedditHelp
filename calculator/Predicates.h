// Predicates.h — Character classification functors for the lexer
//
// Each predicate is a callable struct that tests a single character against
// a condition. They are designed to be passed by value into Lexer::test()
// and Lexer::skip(), which consume characters while the predicate holds.
//
// Note: isidentifier is stateful — it uses a counter to enforce that the
// first character must be alpha/underscore while subsequent characters may
// also be digits. A fresh instance is created on each call to skip().

#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

namespace Interpreter {

// Matches C-style identifiers: first char is [a-zA-Z_], rest are [a-zA-Z0-9_].
// Stateful: tracks position via an internal counter.
struct isidentifier {
    bool operator()(char chr) {
        // First character: must be alpha or underscore
        if (counter++ == 0) {
            return (chr == '_') || (std::isalpha(chr) != 0);
        }
        // Subsequent characters: also allow digits
        return (chr == '_') || (std::isalnum(chr) != 0);
    }
    int counter = 0;
};

// Matches any alphabetic character [a-zA-Z].
struct isalpha {
    bool operator()(char chr) {
        return (std::isalpha(chr) != 0);
    }
};

// Matches any digit [0-9].
struct isdigit {
    bool operator()(char chr) {
        return (std::isdigit(chr) != 0);
    }
};

// Matches any alphanumeric character [a-zA-Z0-9].
struct isalnum {
    bool operator()(char chr) {
        return (std::isalnum(chr) != 0);
    }
};

// Matches any whitespace character (space, tab, newline, etc.).
struct isspace {
    bool operator()(char chr) {
        return (std::isspace(chr) != 0);
    }
};

// Matches a specific single character.
struct ischar {
    ischar(char val) : chr(val) {
    }
    bool operator()(char val) const {
        return (val == chr);
    }
    char chr;
};

// Matches any character present in the given string_view.
struct isany {
    isany(std::string_view str) : _str(str) {
    }
    bool operator()(char chr) const {
        return std::any_of(_str.begin(), _str.end(), [chr](char val) { return val == chr; });
    }
    std::string_view _str;
};

}  // namespace Interpreter

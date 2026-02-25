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
    bool operator()(char ch) {
        // First character: must be alpha or underscore
        if (counter++ == 0) {
            return (ch == '_') || (std::isalpha(ch) != 0);
        }
        // Subsequent characters: also allow digits
        return (ch == '_') || (std::isalnum(ch) != 0);
    }
    int counter = 0;
};

// Matches any alphabetic character [a-zA-Z].
struct isalpha {
    bool operator()(char ch) {
        return (std::isalpha(ch) != 0);
    }
};

// Matches any digit [0-9].
struct isdigit {
    bool operator()(char ch) {
        return (std::isdigit(ch) != 0);
    }
};

// Matches any alphanumeric character [a-zA-Z0-9].
struct isalnum {
    bool operator()(char ch) {
        return (std::isalnum(ch) != 0);
    }
};

// Matches any whitespace character (space, tab, newline, etc.).
struct isspace {
    bool operator()(char ch) {
        return (std::isspace(ch) != 0);
    }
};

// Matches a specific single character.
struct ischar {
    ischar(char ch) : c(ch) {
    }
    bool operator()(char ch) const {
        return (ch == c);
    }
    char c;
};

// Matches any character present in the given string_view.
struct isany {
    isany(std::string_view sv) : _sv(sv) {
    }
    bool operator()(char ch) const {
        return std::any_of(_sv.begin(), _sv.end(), [ch](char c) { return c == ch; });
    }
    std::string_view _sv;
};

}  // namespace Interpreter

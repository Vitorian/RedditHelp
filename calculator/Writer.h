// Writer.h — Visitor-based serialization of AST nodes
//
// Implements a simple Visitor that accumulates output into a character buffer.
// The visit(Node*) method is intentionally a no-op; subclasses or callers
// are expected to dynamic_cast the node to determine its concrete type and
// call write() with the appropriate representation.
//
// This serves as a minimal example of the Visitor pattern applied to the
// expression tree. A full implementation would override visit() to inspect
// each node type and produce formatted output.

#pragma once

#include "TreeNodes.h"

#include <cstdio>
#include <cstring>
#include <string_view>
#include <vector>

namespace Interpreter {

// Writer — accumulates text into a flat char buffer via the Visitor interface.
struct Writer : public Visitor {

    // Character buffer holding the serialized output.
    std::vector<char> data;

    // Called for each visited node. Currently a no-op; serves as the hook
    // point for subclasses to dispatch on concrete node types.
    void visit(Node*) override {
    }

    // Formats a double value and appends it to the buffer.
    void write(double value) {
        char buf[64];
        int len = ::snprintf(buf, sizeof(buf), "%f", value);
        if (len > 0) {
            write(std::string_view(buf, static_cast<size_t>(len)));
        }
    }

    // Appends raw text (string_view) to the buffer via memcpy.
    void write(std::string_view sv) {
        size_t size = data.size();
        data.resize(size + sv.size());
        memcpy(&data[size], sv.data(), sv.size());
    }
};

}  // namespace Interpreter

// Node.h — Abstract base class for all AST nodes
//
// Every node in the expression tree inherits from Node and implements calc()
// to evaluate its subtree. The Visitor interface enables tree traversal for
// serialization, pretty-printing, or other AST transformations.

#pragma once

#include "Pointer.h"

namespace Interpreter {

struct Node;

// Visitor — interface for traversing the AST without modifying node classes.
struct Visitor : public RefCounted {
    virtual void visit(Node*) = 0;
};

// Node — base class for all expression tree nodes.
// Subclasses implement calc() to return the evaluated result of their subtree.
struct Node : public RefCounted {
    using RefCounted::RefCounted;
    virtual ~Node() {
    }

    // Evaluates this node and its children, returning the numeric result.
    virtual double calc() = 0;

    // Accepts a visitor for tree traversal (default: visits self).
    virtual void visit(Visitor& visitor) {
        visitor.visit(this);
    }
};

}  // namespace Interpreter

// TreeNodes.h — Concrete AST node types for the expression tree
//
// Defines all node types that the parser can produce. Each node implements
// calc() to evaluate itself, and optionally visit() for tree traversal.
//
// Node hierarchy:
//   Node (abstract)
//   ├── Constant       — literal numeric value
//   ├── Parenthesis    — grouping wrapper, delegates to inner node
//   ├── UnaryOp        — prefix +/- applied to a single operand
//   ├── BinaryOp       — infix +, -, *, / with two operands
//   ├── Variable       — named value, looked up from a symbol table
//   └── FunctionCall   — base for function invocations
//       └── FunctionCallWithArgs<N> — N-argument function call (template)
//
// Also defines Function, a non-node descriptor that maps a name and arity
// to a type-erased function pointer (FnPtr).

#pragma once

#include "Pointer.h"
#include "Node.h"
#include "FunctionOps.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Interpreter {

// Convenience alias used throughout the parser and tree manipulation code.
using NodePtr = Pointer<Node>;

// Constant — a leaf node holding a literal floating-point value.
struct Constant : public Node {
    Constant(double dval) {
        value = dval;
    }
    double value;

    // Returns the stored literal value.
    double calc() override {
        return value;
    }
};

// Parenthesis — a transparent wrapper that preserves grouping in the AST.
// Evaluates to whatever its inner expression evaluates to.
struct Parenthesis : public Node {
    Parenthesis(NodePtr n) {
        node = std::move(n);
    }
    NodePtr node;

    // Delegates evaluation to the enclosed expression.
    double calc() override {
        return node->calc();
    }

    // Visits self first, then the inner node.
    void visit(Visitor& visitor) override {
        visitor.visit(this);
        visitor.visit(node.get());
    }
};

// UnaryOp — applies a prefix sign operator (+/-) to a single operand.
struct UnaryOp : public Node {
    enum class Operation : uint16_t { NA = 0, Negative = 1, Positive = 2 };
    Operation op = Operation::NA;
    NodePtr node;

    // Negates or passes through the operand based on the sign.
    double calc() override {
        switch (op) {
            case Operation::Positive: return node->calc();
            case Operation::Negative: return -node->calc();
            case Operation::NA: return node->calc();
        }
        return node->calc();
    }
};

// BinaryOp — an interior node representing an infix arithmetic operation.
// Holds two child nodes (left, right) and the operator connecting them.
struct BinaryOp : public Node {
    enum class Operation : int { NA, Addition, Subtraction, Multiplication, Division };

    // Returns the precedence level of an operator.
    // Higher values bind more tightly: mul/div (2) > add/sub (1) > NA (0).
    static int precedence(Operation op) {
        switch (op) {
            case Operation::NA: return 0;
            case Operation::Addition:
            case Operation::Subtraction: return 1;
            case Operation::Multiplication:
            case Operation::Division: return 2;
        }
        return 3;
    }

    BinaryOp(Operation oper, NodePtr lhs, NodePtr rhs) {
        op = oper;
        left = std::move(lhs);
        right = std::move(rhs);
    }

    // Recursively evaluates both children and applies the operator.
    // Division by zero follows IEEE 754 semantics (returns ±inf or NaN).
    double calc() override {
        switch (op) {
            case Operation::Addition: return left->calc() + right->calc();
            case Operation::Subtraction: return left->calc() - right->calc();
            case Operation::Multiplication: return left->calc() * right->calc();
            case Operation::Division: return left->calc() / right->calc();
            case Operation::NA: break;
        }
        return 0;
    }

    // In-order traversal: left child, self, right child.
    void visit(Visitor& visitor) override {
        visitor.visit(left.get());
        visitor.visit(this);
        visitor.visit(right.get());
    }

    Operation op = Operation::NA;
    NodePtr left;
    NodePtr right;
};

// Variable — a named leaf node whose value can be assigned externally.
// The parser interns variables in a map so repeated references share one node.
struct Variable : public Node {
    Variable(std::string_view varname) {
        name = varname;
    }
    std::string name;
    double value = 0.0;

    // Returns the currently assigned value.
    double calc() override {
        return value;
    }
    void visit(Visitor& visitor) override {
        visitor.visit(this);
    }
};

// Function — a descriptor (not a node) that maps a function name and arity
// to a type-erased function pointer. Stored in Calculator's function map.
struct Function {
    std::string name;
    size_t num_args;
    FnPtr fnptr;
};

// FunctionCall — abstract base for all function call nodes.
// The actual implementation lives in the FunctionCallWithArgs<N> template.
struct FunctionCall : public Node {};

// FunctionCallWithArgs<N> — a function call node with exactly N arguments.
// Stores arguments in a fixed-size array and dispatches through callfn().
template <size_t N>
struct FunctionCallWithArgs : public FunctionCall {
    FunctionCallWithArgs(FnPtr fn, const std::vector<NodePtr>& arguments) {
        fnptr = fn;
        // Copy the first N argument nodes from the parser's dynamic vector
        // into the fixed-size array.
        for (unsigned j = 0; j < N; ++j) {
            args[j] = arguments[j];
        }
    }

    std::array<NodePtr, N> args;  // argument expression nodes
    FnPtr fnptr;                  // type-erased pointer to the C function

    // Evaluates all argument expressions into a values array, then dispatches
    // the function call through callfn() which reinterpret_casts to the
    // correct N-argument signature.
    double calc() override {
        std::array<double, N> values;
        for (size_t j = 0; j < N; ++j) {
            values[j] = args[j]->calc();
        }
        double value = callfn(fnptr, values.data(), N);
        return value;
    }

    // Visits self first, then each argument node in order.
    void visit(Visitor& visitor) override {
        visitor.visit(this);
        for (const NodePtr& arg : args) {
            visitor.visit(arg.get());
        }
    }
};

}  // namespace Interpreter

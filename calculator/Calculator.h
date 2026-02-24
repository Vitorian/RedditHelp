// Calculator.h — Recursive descent parser for arithmetic expressions
//
// Implements the high-level grammar rules on top of Lexer's character-level
// primitives. The parser produces an AST (tree of Node subclasses) that can
// be evaluated by calling calc() on the root.
//
// Grammar (informal):
//   expression  = primitive (arithop expression)*
//   primitive   = variable | parenthesis | dbl64
//   parenthesis = '(' expression ')'
//   variable    = identifier
//   function    = identifier '(' expression (',' expression)* ')'
//   dbl64       = [+-]? digits ['.' digits]?
//
// Operator precedence is handled post-parse by adjustPrecedence(), which
// rotates the tree so that higher-precedence operators end up deeper,
// ensuring correct evaluation order without an explicit precedence-climbing
// or Pratt parser.
//
// Note: function() is defined but not yet wired into primitive(), so
// function calls like log(10) are currently parsed as variable references.

#include <cstdint>
#include <cstdio>
#include <string_view>

#include "Pointer.h"
#include "TreeNodes.h"
#include "Predicates.h"
#include "Lexer.h"
#include "FunctionOps.h"

namespace Interpreter {

// Calculator — the full expression parser.
// Inherits Lexer's scanning primitives and adds grammar-level productions.
struct Calculator : public Lexer {

    // Parses a floating-point literal and wraps it in a Constant node.
    NodePtr dbl64() {
        if (auto dbl = parsedouble()) {
            return NodePtr(new Constant(dbl.value()));
        }
        return NodePtr();
    }

    // Parses a parenthesized sub-expression: '(' expression ')'.
    // Returns a Parenthesis node wrapping the inner expression, or null
    // if the input doesn't match this production.
    NodePtr parenthesis() {
        StackSaver saver(this);
        if (test(ischar('('))) {
            if (auto xp = expression()) {
                if (test(ischar(')'))) {
                    saver.commit();
                    return NodePtr(new Parenthesis(xp));
                }
            }
        }
        return NodePtr();
    }

    // Parses the lowest-level value-producing construct: a variable name,
    // a parenthesized expression, or a numeric literal.
    // Tries each alternative in order using short-circuit evaluation.
    NodePtr primitive() {
        StackSaver saver(this);
        NodePtr lhs;
        if ((lhs = variable()) || (lhs = parenthesis()) || (lhs = dbl64())) {
            saver.commit();
        }
        return lhs;
    }

    // Fixes operator precedence by rotating the tree when necessary.
    // Called after parsing "lhs OP rhs" where rhs may itself be a BinaryOp.
    //
    // If rhs is a BinaryOp with lower precedence than OP, we rotate:
    //   Before:  lhs OP (rhs_left RHSOP rhs_right)   [wrong: RHSOP binds looser]
    //   After:   (lhs OP rhs_left) RHSOP rhs_right   [correct: OP binds tighter]
    //
    // Otherwise, we simply create: (lhs OP rhs).
    void adjustPrecedence(NodePtr& lhs, NodePtr& rhs, BinaryOp::Operation op) {
        auto binop = rhs.as<BinaryOp>();
        if (binop && (BinaryOp::precedence(binop->op) < BinaryOp::precedence(op))) {
            // Steal rhs's left child as our right operand, push ourselves down.
            binop->left.reset(new BinaryOp(op, lhs, binop->left));
            lhs = rhs;
        } else {
            lhs.reset(new BinaryOp(op, lhs, rhs));
        }
    }

    // Parses a full expression: a primitive optionally followed by one or more
    // "operator expression" pairs. Builds the AST left-to-right, then relies
    // on adjustPrecedence() to rotate nodes for correct operator binding.
    NodePtr expression() {
        StackSaver saver(this);
        skipws();
        if (auto lhs = primitive()) {
            // Greedily consume "op rhs" pairs.
            while (auto op = arithop()) {
                if (auto rhs = expression()) {
                    adjustPrecedence(lhs, rhs, op.value());
                }
            }
            saver.commit();
            return lhs;
        }
        return {};
    }

    // Parses a variable name (identifier) and interns it in the variable map.
    // Repeated references to the same name return the same Variable node,
    // so assigning a value to "x" is visible to all occurrences.
    NodePtr variable() {
        StackSaver saver(this);
        if (auto name = skip(isidentifier())) {
            // Look up or create the Variable in the interning map.
            Pointer<Variable>& var(_variable_map[std::string(name.value())]);
            if (!var) {
                var = new Variable(name.value());
            }
            saver.commit();
            return var;
        }
        return {};
    }

    // Creates the correct FunctionCallWithArgs<N> node for a given function
    // name and argument list. Returns null if the function is unknown or if
    // the argument count doesn't match the function's declared arity.
    // Uses a switch to select the right template instantiation at runtime.
    Pointer<FunctionCall> createFunctionCall(std::string_view name,
                                             const std::vector<NodePtr>& args) {
        if (auto fn = findFunction(name)) {
            if (args.size() == fn->num_args) {
                switch (fn->num_args) {
                    case 0:  return Pointer<FunctionCall>(new FunctionCallWithArgs<0>(fn->fnptr, args));
                    case 1:  return Pointer<FunctionCall>(new FunctionCallWithArgs<1>(fn->fnptr, args));
                    case 2:  return Pointer<FunctionCall>(new FunctionCallWithArgs<2>(fn->fnptr, args));
                    case 3:  return Pointer<FunctionCall>(new FunctionCallWithArgs<3>(fn->fnptr, args));
                    case 4:  return Pointer<FunctionCall>(new FunctionCallWithArgs<4>(fn->fnptr, args));
                    case 5:  return Pointer<FunctionCall>(new FunctionCallWithArgs<5>(fn->fnptr, args));
                    case 6:  return Pointer<FunctionCall>(new FunctionCallWithArgs<6>(fn->fnptr, args));
                    case 7:  return Pointer<FunctionCall>(new FunctionCallWithArgs<7>(fn->fnptr, args));
                    case 8:  return Pointer<FunctionCall>(new FunctionCallWithArgs<8>(fn->fnptr, args));
                    case 9:  return Pointer<FunctionCall>(new FunctionCallWithArgs<9>(fn->fnptr, args));
                    case 10: return Pointer<FunctionCall>(new FunctionCallWithArgs<10>(fn->fnptr, args));
                }
            }
        }
        return {};
    }

    // Parses a function call: identifier '(' expression (',' expression)* ')'.
    // Collects arguments into a vector, then delegates to createFunctionCall().
    // Note: not yet called from primitive(), so this production is unreachable.
    Pointer<FunctionCall> function() {
        StackSaver saver(this);
        if (auto name = skip(isidentifier())) {
            if (test(ischar('('))) {
                std::vector<NodePtr> args;
                // Parse the first argument (if any).
                if (auto xp = expression()) {
                    args.push_back(xp);
                    // Parse subsequent comma-separated arguments.
                    while (test(ischar(','))) {
                        if (auto next = expression()) {
                            args.push_back(next);
                        }
                    }
                }
                if (test(ischar(')'))) {
                    if (auto fn = createFunctionCall(name.value(), args)) {
                        saver.commit();
                        return fn;
                    }
                }
            }
        }
        return {};
    }

    // Looks up a function by name in the function map.
    // Returns a copy of the Function descriptor, or empty if not found.
    std::optional<Function> findFunction(std::string_view name) {
        auto fn = _function_map.find(std::string(name));
        if (fn != _function_map.end()) return fn->second;
        return {};
    }

    // Symbol table for variables. Shared references ensure that assigning
    // to a variable is visible wherever that variable appears in the AST.
    using VariableMap = std::unordered_map<std::string, Pointer<Variable>>;
    VariableMap _variable_map;

    // Registry of callable functions. Pre-populated with log() as an example.
    // To add more functions: insert Function{name, arity, reinterpret_cast<FnPtr>(&fn)}.
    using FunctionMap = std::unordered_map<std::string, Function>;
    FunctionMap _function_map = {{"log", Function{"log", 1, reinterpret_cast<FnPtr>(&::log)}}};

    // Entry point: resets the lexer to the given input and parses a full expression.
    NodePtr parse(std::string_view code) {
        reset(code);
        return expression();
    };
};

}  // namespace Interpreter

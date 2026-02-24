// calculator_test.cpp — Google Test unit tests for the calculator library
//
// Tests cover all layers: smart pointers, character predicates, lexer
// primitives, AST nodes, function dispatch, the full parser, and the writer.

#include "Calculator.h"
#include "Writer.h"

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <string>

using namespace Interpreter;

// ===== Pointer.h — RefCounted & Pointer<T> =====

namespace {

struct TestNode : public Node {
    double calc() override { return 42.0; }
};

struct DerivedNode : public TestNode {
    double calc() override { return 99.0; }
};

}  // namespace

TEST(Pointer, RefCountAddRelease) {
    auto* raw = new TestNode;
    EXPECT_EQ(raw->_counter, 0);

    intrusive_ptr_add_ref(raw);
    EXPECT_EQ(raw->_counter, 1);

    intrusive_ptr_add_ref(raw);
    EXPECT_EQ(raw->_counter, 2);

    intrusive_ptr_release(raw);  // counter 2 → 1, object alive
    EXPECT_EQ(raw->_counter, 1);

    intrusive_ptr_release(raw);  // counter 1 → delete
    // raw is now deleted; nothing to assert but we reached here without crash
}

TEST(Pointer, ScopeDestruction) {
    auto* raw = new TestNode;
    {
        Pointer<TestNode> p(raw);
        EXPECT_EQ(raw->_counter, 1);
        {
            Pointer<TestNode> p2(p);
            EXPECT_EQ(raw->_counter, 2);
        }
        EXPECT_EQ(raw->_counter, 1);
    }
    // raw deleted here; reaching this line means no double-free
}

TEST(Pointer, DynamicDowncastSuccess) {
    Pointer<Node> base(new DerivedNode);
    auto derived = base.as<DerivedNode>();
    ASSERT_TRUE(derived);
    EXPECT_DOUBLE_EQ(derived->calc(), 99.0);
}

TEST(Pointer, DynamicDowncastFailure) {
    Pointer<Node> base(new TestNode);
    auto derived = base.as<DerivedNode>();
    EXPECT_FALSE(derived);
}

// ===== Predicates.h =====

TEST(Predicates, IsIdentifier) {
    isidentifier pred;
    // First char: alpha/underscore accepted
    EXPECT_TRUE(pred('a'));
    // Subsequent: digits allowed
    EXPECT_TRUE(pred('1'));
    EXPECT_TRUE(pred('_'));
}

TEST(Predicates, IsIdentifierRejectsLeadingDigit) {
    isidentifier pred;
    EXPECT_FALSE(pred('0'));
}

TEST(Predicates, IsDigit) {
    Interpreter::isdigit pred;
    EXPECT_TRUE(pred('0'));
    EXPECT_TRUE(pred('9'));
    EXPECT_FALSE(pred('a'));
    EXPECT_FALSE(pred(' '));
}

TEST(Predicates, IsAlpha) {
    Interpreter::isalpha pred;
    EXPECT_TRUE(pred('a'));
    EXPECT_TRUE(pred('Z'));
    EXPECT_FALSE(pred('0'));
    EXPECT_FALSE(pred('_'));
}

TEST(Predicates, IsAlnum) {
    Interpreter::isalnum pred;
    EXPECT_TRUE(pred('a'));
    EXPECT_TRUE(pred('5'));
    EXPECT_FALSE(pred('!'));
}

TEST(Predicates, IsSpace) {
    Interpreter::isspace pred;
    EXPECT_TRUE(pred(' '));
    EXPECT_TRUE(pred('\t'));
    EXPECT_TRUE(pred('\n'));
    EXPECT_FALSE(pred('x'));
}

TEST(Predicates, IsChar) {
    ischar pred('+');
    EXPECT_TRUE(pred('+'));
    EXPECT_FALSE(pred('-'));
}

TEST(Predicates, IsAny) {
    isany pred("+-*/");
    EXPECT_TRUE(pred('+'));
    EXPECT_TRUE(pred('/'));
    EXPECT_FALSE(pred('x'));
    EXPECT_FALSE(pred(' '));
}

// ===== Lexer.h =====

// Helper: create a Lexer, reset it to input, and return it.
static Lexer makeLexer(std::string_view input) {
    Lexer lex;
    lex.reset(input);
    return lex;
}

TEST(Lexer, StackSaverCommit) {
    Lexer lex = makeLexer("abc");
    {
        Lexer::StackSaver saver(&lex);
        lex.test(ischar('a'));
        saver.commit();
    }
    // After commit, position is advanced past 'a'
    auto ch = lex.test(ischar('b'));
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'b');
}

TEST(Lexer, StackSaverRestore) {
    Lexer lex = makeLexer("abc");
    {
        Lexer::StackSaver saver(&lex);
        lex.test(ischar('a'));
        // No commit → destructor restores
    }
    // Position should be back at 'a'
    auto ch = lex.test(ischar('a'));
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), 'a');
}

TEST(Lexer, TestMatch) {
    Lexer lex = makeLexer("+x");
    auto ch = lex.test(ischar('+'));
    ASSERT_TRUE(ch.has_value());
    EXPECT_EQ(ch.value(), '+');
    // Iterator advanced: next char is 'x'
    auto ch2 = lex.test(ischar('x'));
    ASSERT_TRUE(ch2.has_value());
}

TEST(Lexer, TestNoMatch) {
    Lexer lex = makeLexer("x");
    auto ch = lex.test(ischar('+'));
    EXPECT_FALSE(ch.has_value());
}

TEST(Lexer, Skip) {
    Lexer lex = makeLexer("   abc");
    auto ws = lex.skip(Interpreter::isspace());
    ASSERT_TRUE(ws.has_value());
    EXPECT_EQ(ws.value(), "   ");
}

TEST(Lexer, ParseUint) {
    Lexer lex = makeLexer("123abc");
    auto val = lex.parseuint();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 123u);
}

TEST(Lexer, ParseUintNonDigit) {
    Lexer lex = makeLexer("abc");
    auto val = lex.parseuint();
    EXPECT_FALSE(val.has_value());
}

TEST(Lexer, ParseDoubleInteger) {
    Lexer lex = makeLexer("42");
    auto val = lex.parsedouble();
    ASSERT_TRUE(val.has_value());
    EXPECT_DOUBLE_EQ(val.value(), 42.0);
}

TEST(Lexer, ParseDoubleDecimal) {
    Lexer lex = makeLexer("3.14");
    auto val = lex.parsedouble();
    ASSERT_TRUE(val.has_value());
    EXPECT_NEAR(val.value(), 3.14, 1e-9);
}

TEST(Lexer, ParseDoubleSigned) {
    Lexer lex = makeLexer("-5");
    auto val = lex.parsedouble();
    ASSERT_TRUE(val.has_value());
    EXPECT_DOUBLE_EQ(val.value(), -5.0);

    Lexer lex2 = makeLexer("+7");
    auto val2 = lex2.parsedouble();
    ASSERT_TRUE(val2.has_value());
    EXPECT_DOUBLE_EQ(val2.value(), 7.0);
}

TEST(Lexer, ParseDoubleEmpty) {
    Lexer lex = makeLexer("abc");
    auto val = lex.parsedouble();
    EXPECT_FALSE(val.has_value());
}

TEST(Lexer, ArithOp) {
    auto check = [](const char* input, BinaryOp::Operation expected) {
        Lexer lex = makeLexer(input);
        auto op = lex.arithop();
        ASSERT_TRUE(op.has_value()) << "input: " << input;
        EXPECT_EQ(op.value(), expected) << "input: " << input;
    };
    check("+", BinaryOp::Operation::Addition);
    check("-", BinaryOp::Operation::Subtraction);
    check("*", BinaryOp::Operation::Multiplication);
    check("/", BinaryOp::Operation::Division);
}

TEST(Lexer, SkipWs) {
    Lexer lex = makeLexer("  \t\n42");
    lex.skipws();
    auto val = lex.parseuint();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 42u);
}

// ===== TreeNodes.h =====

TEST(TreeNodes, ConstantCalc) {
    Pointer<Constant> c(new Constant(7.5));
    EXPECT_DOUBLE_EQ(c->calc(), 7.5);
}

TEST(TreeNodes, ParenthesisCalc) {
    NodePtr inner(new Constant(3.0));
    Pointer<Parenthesis> p(new Parenthesis(inner));
    EXPECT_DOUBLE_EQ(p->calc(), 3.0);
}

TEST(TreeNodes, UnaryOpPositive) {
    auto* u = new UnaryOp;
    u->op = UnaryOp::Operation::Positive;
    u->node = NodePtr(new Constant(10.0));
    Pointer<UnaryOp> ptr(u);
    EXPECT_DOUBLE_EQ(ptr->calc(), 10.0);
}

TEST(TreeNodes, UnaryOpNegative) {
    auto* u = new UnaryOp;
    u->op = UnaryOp::Operation::Negative;
    u->node = NodePtr(new Constant(10.0));
    Pointer<UnaryOp> ptr(u);
    EXPECT_DOUBLE_EQ(ptr->calc(), -10.0);
}

TEST(TreeNodes, UnaryOpNA) {
    auto* u = new UnaryOp;
    u->op = UnaryOp::Operation::NA;
    u->node = NodePtr(new Constant(10.0));
    Pointer<UnaryOp> ptr(u);
    EXPECT_DOUBLE_EQ(ptr->calc(), 10.0);
}

TEST(TreeNodes, BinaryOpAllFour) {
    auto make = [](BinaryOp::Operation op, double a, double b) {
        return Pointer<BinaryOp>(new BinaryOp(op, NodePtr(new Constant(a)),
                                              NodePtr(new Constant(b))));
    };
    EXPECT_DOUBLE_EQ(make(BinaryOp::Operation::Addition, 2, 3)->calc(), 5.0);
    EXPECT_DOUBLE_EQ(make(BinaryOp::Operation::Subtraction, 10, 4)->calc(), 6.0);
    EXPECT_DOUBLE_EQ(make(BinaryOp::Operation::Multiplication, 6, 7)->calc(), 42.0);
    EXPECT_DOUBLE_EQ(make(BinaryOp::Operation::Division, 10, 4)->calc(), 2.5);
}

TEST(TreeNodes, BinaryOpPrecedence) {
    using Op = BinaryOp::Operation;
    EXPECT_LT(BinaryOp::precedence(Op::NA), BinaryOp::precedence(Op::Addition));
    EXPECT_EQ(BinaryOp::precedence(Op::Addition), BinaryOp::precedence(Op::Subtraction));
    EXPECT_GT(BinaryOp::precedence(Op::Multiplication), BinaryOp::precedence(Op::Addition));
    EXPECT_EQ(BinaryOp::precedence(Op::Multiplication), BinaryOp::precedence(Op::Division));
}

TEST(TreeNodes, Variable) {
    Pointer<Variable> v(new Variable("x"));
    EXPECT_DOUBLE_EQ(v->calc(), 0.0);  // default
    v->value = 42.0;
    EXPECT_DOUBLE_EQ(v->calc(), 42.0);
    EXPECT_EQ(v->name, "x");
}

TEST(TreeNodes, FunctionCallWithArgs) {
    // Use a simple 1-arg function
    auto square = [](double x) -> double { return x * x; };
    FnPtr fn = reinterpret_cast<FnPtr>(+square);  // + converts lambda to fn ptr

    std::vector<NodePtr> args = {NodePtr(new Constant(5.0))};
    Pointer<FunctionCallWithArgs<1>> call(new FunctionCallWithArgs<1>(fn, args));
    EXPECT_DOUBLE_EQ(call->calc(), 25.0);
}

// ===== FunctionOps.h =====

TEST(FunctionOps, CallFn0Args) {
    auto fn0 = []() -> double { return 99.0; };
    FnPtr fp = reinterpret_cast<FnPtr>(+fn0);
    double result = callfn(fp, nullptr, 0);
    EXPECT_DOUBLE_EQ(result, 99.0);
}

TEST(FunctionOps, CallFn1Arg) {
    auto fn1 = [](double x) -> double { return x * 2; };
    FnPtr fp = reinterpret_cast<FnPtr>(+fn1);
    double args[] = {5.0};
    double result = callfn(fp, args, 1);
    EXPECT_DOUBLE_EQ(result, 10.0);
}

TEST(FunctionOps, CallFn2Args) {
    auto fn2 = [](double a, double b) -> double { return a + b; };
    FnPtr fp = reinterpret_cast<FnPtr>(+fn2);
    double args[] = {3.0, 4.0};
    double result = callfn(fp, args, 2);
    EXPECT_DOUBLE_EQ(result, 7.0);
}

TEST(FunctionOps, CallFn3Args) {
    auto fn3 = [](double a, double b, double c) -> double { return a * b + c; };
    FnPtr fp = reinterpret_cast<FnPtr>(+fn3);
    double args[] = {2.0, 3.0, 1.0};
    double result = callfn(fp, args, 3);
    EXPECT_DOUBLE_EQ(result, 7.0);
}

TEST(FunctionOps, UnsupportedSizeReturnsNaN) {
    double result = callfn(nullptr, nullptr, 99);
    EXPECT_TRUE(std::isnan(result));
}

// ===== Calculator.h — full integration =====

TEST(Calculator, SimpleInteger) {
    Calculator calc;
    auto ast = calc.parse("42");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 42.0);
}

TEST(Calculator, Addition) {
    Calculator calc;
    auto ast = calc.parse("2+3");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 5.0);
}

TEST(Calculator, Subtraction) {
    Calculator calc;
    auto ast = calc.parse("10-4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 6.0);
}

TEST(Calculator, Multiplication) {
    Calculator calc;
    auto ast = calc.parse("6*7");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 42.0);
}

TEST(Calculator, Division) {
    Calculator calc;
    auto ast = calc.parse("10/4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 2.5);
}

TEST(Calculator, PrecedenceMulOverAdd) {
    Calculator calc;
    auto ast = calc.parse("2+3*4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 14.0);
}

TEST(Calculator, PrecedenceMulBeforeAdd) {
    Calculator calc;
    auto ast = calc.parse("2*3+4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 10.0);
}

TEST(Calculator, Parentheses) {
    Calculator calc;
    auto ast = calc.parse("(2+3)*4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 20.0);
}

TEST(Calculator, NestedParentheses) {
    Calculator calc;
    auto ast = calc.parse("((2+3))*4");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 20.0);
}

TEST(Calculator, Variables) {
    Calculator calc;
    auto ast = calc.parse("x+1");
    ASSERT_TRUE(ast);
    calc._variable_map["x"]->value = 5.0;
    EXPECT_DOUBLE_EQ(ast->calc(), 6.0);
}

TEST(Calculator, SharedVariableReferences) {
    Calculator calc;
    auto ast = calc.parse("x+x");
    ASSERT_TRUE(ast);
    calc._variable_map["x"]->value = 3.0;
    EXPECT_DOUBLE_EQ(ast->calc(), 6.0);
}

TEST(Calculator, LeadingWhitespace) {
    Calculator calc;
    auto ast = calc.parse(" 2+3");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 5.0);
}

TEST(Calculator, DecimalNumber) {
    Calculator calc;
    auto ast = calc.parse("3.14");
    ASSERT_TRUE(ast);
    EXPECT_NEAR(ast->calc(), 3.14, 1e-9);
}

TEST(Calculator, DecimalArithmetic) {
    Calculator calc;
    auto ast = calc.parse("0.5+0.5");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 1.0);
}

TEST(Calculator, NegativeNumber) {
    Calculator calc;
    auto ast = calc.parse("-5+10");
    ASSERT_TRUE(ast);
    EXPECT_DOUBLE_EQ(ast->calc(), 5.0);
}

TEST(Calculator, DivisionByZero) {
    Calculator calc;
    auto ast = calc.parse("1/0");
    ASSERT_TRUE(ast);
    EXPECT_TRUE(std::isinf(ast->calc()));
}

TEST(Calculator, ParseFailureReturnsNull) {
    Calculator calc;
    auto ast = calc.parse("");
    EXPECT_FALSE(ast);
}

// ===== Writer.h =====

TEST(Writer, WriteDouble) {
    Writer w;
    w.write(3.14);
    std::string result(w.data.begin(), w.data.end());
    EXPECT_NE(result.find("3.14"), std::string::npos);
}

TEST(Writer, WriteStringView) {
    Writer w;
    w.write(std::string_view("hello"));
    std::string result(w.data.begin(), w.data.end());
    EXPECT_EQ(result, "hello");
}

TEST(Writer, WriteAppends) {
    Writer w;
    w.write(std::string_view("ab"));
    w.write(std::string_view("cd"));
    std::string result(w.data.begin(), w.data.end());
    EXPECT_EQ(result, "abcd");
}

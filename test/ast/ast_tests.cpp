#include <gtest/gtest.h>

#include <unordered_map>
#include <cstddef>
#include <memory>
#include <exception>

#include "ast/Expr.hpp"
#include "visit/SmallVisitors.hpp"

class Evaluator final : ast::Visitor<int> {
public:

  int
  evaluate(ast::Expr &expr)
  {
    return visit(expr);
  }

  virtual int visitBinOp(ast::BinOp &binOp) override
  {
    return visit(binOp.lhs()) + visit(binOp.rhs());
  }

  virtual int visitUnaryOp(ast::UnaryOp &unaryOp) override
  {
    return visit(unaryOp.child());
  }

  virtual int visitString(ast::String &string) override
  {
    throw std::runtime_error("Cannot evaluate as integer.");
  }

  virtual int visitNum(ast::Num &num) override
  {
    return num.value();
  }

  virtual int visitGrouping(ast::Grouping &grouping) override
  {
    return visit(grouping.child());
  }

};

class IdUniquessChecker final : ast::Visitor<void> {
public:

  bool hasUniqueIds(ast::Expr &expression)
  {
    visit(expression);
    std::sort(ids.begin(), ids.end());
    auto last = std::unique(ids.begin(), ids.end());
    // If nothing has been removed, the one-past-the-end of the unique'd
    // sequence should be the same as for the original sequence.
    return last == ids.end();
  }

  virtual void visitBinOp(ast::BinOp &binOp) override
  {
    ids.push_back(binOp.id());
    visit(binOp.lhs());
    visit(binOp.rhs());
  }

  virtual void visitUnaryOp(ast::UnaryOp &unaryOp) override
  {
    ids.push_back(unaryOp.id());
    visit(unaryOp.child());
  }

  virtual void visitString(ast::String &string) override
  {
    ids.push_back(string.id());
  }

  virtual void visitNum(ast::Num &num) override
  {
    ids.push_back(num.id());
  }

  virtual void visitGrouping(ast::Grouping &grouping) override
  {
    ids.push_back(grouping.id());
    visit(grouping.child());
  }

private:
  std::vector<size_t> ids{};

};

TEST(AstTests, TestEvaluate) {
  Evaluator visitor;
  ast::Expr expr = ast::add(ast::num(4), ast::num(5));

  auto evaluation = visitor.evaluate(expr);

  ASSERT_EQ(9, evaluation);
}

TEST(AstTests, TestUniqueIds) {
  IdUniquessChecker checker;
  using namespace ast;
  Expr expr = sub(num(1), mult(num(3), num(5)));

  ASSERT_TRUE(checker.hasUniqueIds(expr));
}

#pragma once

#include <string>
#include <utility>
#include <sstream>

#include "ast/Expr.hpp"

namespace visit {

class PrettyPrinter final : ast::ConstVisitor<void> {
public:

  std::string
  print(const ast::Expr &expression)
  {
    output.clear();
    visit(expression);
    return std::move(output);
  }

  virtual void visitBinOp(const ast::BinOp &binOp) override
  {
    output.append("(");
    switch(binOp.operation()) {
      case ast::BinOp::Op::Add:
        output.append("+"); break;
      case ast::BinOp::Op::Div:
        output.append("/"); break;
      case ast::BinOp::Op::Eq:
        output.append("=="); break;
      case ast::BinOp::Op::Gt:
        output.append(">"); break;
      case ast::BinOp::Op::GtEq:
        output.append(">="); break;
      case ast::BinOp::Op::Lt:
        output.append("<"); break;
      case ast::BinOp::Op::LtEq:
        output.append("<="); break;
      case ast::BinOp::Op::Mult:
        output.append("*"); break;
      case ast::BinOp::Op::Neq:
        output.append("!="); break;
      case ast::BinOp::Op::Sub:
        output.append("-"); break;
    }
    output.append(" ");
    visit(binOp.lhs());
    output.append(" ");
    visit(binOp.rhs());
    output.append(")");
  }

  virtual void visitUnaryOp(const ast::UnaryOp &unaryOp) override
  {
    output.append("(");
    switch(unaryOp.operation()) {
      case ast::UnaryOp::Op::Negate:
        output.append("- "); break;
      case ast::UnaryOp::Op::Nott:
      output.append("¬ "); break;
    }
    visit(unaryOp.child());
    output.append(")");
  }

  virtual void visitString(const ast::String &string) override
  {
    output.append("\"");
    output.append(string.value());
    output.append("\"");
  }

  virtual void visitNum(const ast::Num &num) override
  {
    std::stringstream stream;
    stream.precision(3);
    stream << std::fixed << num.value();
    auto numString = stream.str();

    // Drop trailing zeros and dot (if present).
    numString.erase(numString.find_last_not_of('0') + 1, std::string::npos);
    if (numString.back() == '.') {
      numString.erase(numString.end() - 1, numString.end());
    }

    output.append(numString);
  }

  virtual void visitGrouping(const ast::Grouping &grouping) override
  {
    output.append("(group ");
    visit(grouping.child());
    output.append(")");
  }

  virtual void visitFalsee(const ast::Falsee &f) override
  {
    output.append("false");
  }

  virtual void visitTruee(const ast::Truee &t) override
  {
    output.append("true");
  }

  virtual void visitNil(const ast::Nil &nil) override
  {
    output.append("nil");
  }

private:
  std::string output{};

};

}

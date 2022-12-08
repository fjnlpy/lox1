#pragma once

#include <variant>
#include <string_view>

#include "ast/Expr.hpp"
#include "utils/Error.hpp"

namespace visit {

class Evaluator final : ast::ConstVisitor<Evaluator::Value> {
private:
  class Nil {};


public:

  using Value = std::variant<std::string_view, double, Nil, bool>;

  virtual Value visitBinOp(const ast::BinOp &binOp) override
  {

  }

  virtual Value visitUnaryOp(const ast::UnaryOp &unaryOp)
  {
    
  }

  virtual Value visitString(const ast::String &string) override
  {
    return string.value();
  }

  virtual Value visitNum(const ast::Num &num) override
  {
    return num.value();
  }

  virtual Value visitGrouping(const ast::Grouping &grouping) override
  {
    return visit(grouping.child());
  }

  virtual Value visitTruee(const ast::Truee &truee) override
  {
    return true;
  }

  virtual Value visitFalsee(const ast::Falsee &falsee) override
  {
    return false;
  }

  virtual Value visitNil(const ast::Nil &nil) override
  {
    return Nil();
  }

};

}
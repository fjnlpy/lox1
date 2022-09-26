#include "parser/Parser.h"

#include <memory>
#include <exception>

#include "utils/Counter.hpp"
#include "utils/Assert.hpp"

using lexer::Token;

namespace parser {

ast::Expr
Parser::parse(std::vector<Token> tokens)
{
  current_ = -1;
  tokens_ = std::move(tokens);
 
  return expression();
}

ast::Expr
Parser::expression()
{
  return equality();
}

ast::Expr
Parser::equality()
{
  using ast::BinOp;

  const auto mappingFunc = [](Token::Type lexerToken) {
    switch (lexerToken) {
      case Token::Type::EQ_EQ:
        return BinOp::Op::Eq;
      case Token::Type::BANG_EQ:
        return BinOp::Op::Neq;
      default:
        throw std::runtime_error("Unexpected token type in equality mapping function");
    }
  };

  return createBinOp(mappingFunc, std::mem_fn(&Parser::comparison), Token::Type::EQ_EQ, Token::Type::BANG_EQ);
}

const lexer::Token &
Parser::current() const
{
  return tokens_.at(current_);
}

const lexer::Token &
Parser::advance()
{
  ++current_;
  return current();
}

template <class... T>
bool
Parser::peek(T &&... tokenTypes)
{
  if (current_ + 1 >= tokens_.size()) {
    return false;
  }

  const auto &nextToken = tokens_[current_ + 1];
  // True if the next token matches any of the parameters.
  return ((nextToken.getType() == std::forward<T>(tokenTypes)) || ...);
}

template <class... T>
std::optional<std::reference_wrapper<const Token>>
Parser::match(T &&... tokenTypes)
{
  if (peek(std::forward<T>(tokenTypes)...)) {
    return advance();
  } else {
    return std::nullopt;
  }
}

template <class BinOpMapFunc, class SubExprFunc, class... Ts>
ast::Expr
Parser::createBinOp(const BinOpMapFunc &map, const SubExprFunc &subExpr, Ts &&... tokenTypes)
{
  ast::Expr lhs = subExpr();
  while (auto op = match(std::forward<Ts>(tokenTypes)...)) {
    ast::Expr rhs = subExpr();
    ast::BinOp::Op op2 = map(op->get().getType());
    // Unfortunately we can't use the factory functions easily here because a different
    // function needs to be called depending on the op of the lexer token, which we must
    // map to a BinOp op.
    // This could have been avoided by also adding factory functions that taken in the
    // BinOp::Op argument to decide which type of BinOp to make (then the existing factory
    // functions could call those).
    lhs = std::make_unique<
      ast::BinOp,
      ast::Expr,
      ast::BinOp::Op,
      ast::Expr,
      size_t
    >(
      std::move(lhs),
      std::move(op2),
      std::move(rhs),
      Counter::next()
    );
  }
  return lhs;
}

}

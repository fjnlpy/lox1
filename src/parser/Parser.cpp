#include "parser/Parser.h"

#include <memory>

#include "utils/Counter.hpp"

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

// TODO: some kind of generic helper for this pattern? I think it needs to take
//  a lambda: (lhs, rhs, token::op) -> Expr
// ast::Expr
// Parser::equality()
// {
//   auto lhs = comparison();
//   while (auto op = match(Token::Type::EQ_EQ, Token::Type::BANG_EQ)) {
//     auto rhs = comparison();
//     switch (op->get().getType()) {
//       // TODO: somewhat frustrating amount of duplication. Is it better to
//       //   just create the ast "manually"? Might actually look better than this
//       //   because the creation can be factored out of the switch statement.
//       case Token::Type::EQ_EQ:
//         lhs = ast::eq(std::move(lhs), std::move(rhs)); break;
//       case Token::Type::BANG_EQ:
//         lhs = ast::neq(std::move(lhs), std::move(rhs)); break;
//     }
//   }
//   return lhs;
// }

ast::Expr
Parser::equality()
{
  using ast::BinOp;

  auto lhs = comparison();
  while (auto op = match(Token::Type::EQ_EQ, Token::Type::BANG_EQ)) {
    auto rhs = comparison();

    ast::BinOp::Op op2;
    switch (op->get().getType()) {
      // TODO: somewhat frustrating amount of duplication. Is it better to
      //   just create the ast "manually"? Might actually look better than this
      //   because the creation can be factored out of the switch statement.
      case Token::Type::EQ_EQ:
        op2 = BinOp::Op::Eq; break;
      case Token::Type::BANG_EQ:
        op2 = BinOp::Op::Neq; break;
    }

    lhs = std::make_unique<BinOp, ast::Expr, BinOp::Op, ast::Expr, size_t>(std::move(lhs), op2, std::move(rhs), Counter::next());
  }
  return lhs;
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
Parser::peek(T &&... tokens)
{
  if (current_ + 1 >= tokens_.size()) {
    return false;
  }

  const auto &nextToken = tokens_[current_ + 1];
  // True if the next token matches any of the parameters.
  return ((nextToken.getType() == std::forward<T>(tokens).getType()) || ...);
}

template <class... T>
std::optional<std::reference_wrapper<const Token>>
Parser::match(T &&... tokens)
{
  if (peek(std::forward<T>(tokens)...)) {
    return advance();
  } else {
    return std::nullopt;
  }
}

}

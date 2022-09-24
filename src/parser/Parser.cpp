#include "parser/Parser.h"

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

// TODO: some kind of generic helper for this pattern?
ast::Expr
Parser::equality()
{
  auto expr = comparison();

  // TODO: the equality stuff.
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

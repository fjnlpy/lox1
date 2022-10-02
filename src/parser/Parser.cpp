#include "parser/Parser.h"

#include <memory>
#include <exception>
#include <string>
#include <sstream>

#include "utils/Error.hpp"
#include "utils/Counter.hpp"
#include "utils/Assert.hpp"

using lexer::Token;
using ast::BinOp;
using ast::UnaryOp;

// Ideally this would be more descriptive but it should not happen anyway...
#define DEFAULT_SWITCH_CASE default: throw std::runtime_error("Unexpected token type in mapping function.");

namespace {

constexpr auto ERROR_TAG = "Parser";

}

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
  // Instead of defining these mapping functions everywhere, it would probably be better to have one mapping function
  // per ast enum that converts all valid lexer tokens to their respective value of that enum or throws if that's not possible.
  const auto mappingFunc = [](Token::Type lexerToken) {
    switch (lexerToken) {
      case Token::Type::EQ_EQ: return BinOp::Op::Eq;
      case Token::Type::BANG_EQ: return BinOp::Op::Neq;
      DEFAULT_SWITCH_CASE
    }
  };

  return createBinOp(mappingFunc, [this] { return comparison(); }, Token::Type::EQ_EQ, Token::Type::BANG_EQ);
}

ast::Expr
Parser::comparison()
{
  const auto mappingFunc = [](Token::Type lexerToken) {
    switch (lexerToken) {
      case Token::Type::GT: return BinOp::Op::Gt;
      case Token::Type::GT_EQ: return BinOp::Op::GtEq;
      case Token::Type::LT: return BinOp::Op::Lt;
      case Token::Type::LT_EQ: return BinOp::Op::LtEq;
      DEFAULT_SWITCH_CASE
    }
  };

  return createBinOp(
    mappingFunc,
    [this] { return term(); },
    Token::Type::GT,
    Token::Type::GT_EQ,
    Token::Type::LT,
    Token::Type::LT_EQ
  );
}

ast::Expr
Parser::term()
{
  const auto mappingFunc = [](Token::Type lexerToken) {
    switch (lexerToken) {
      case Token::Type::MINUS: return BinOp::Op::Sub;
      case Token::Type::PLUS: return BinOp::Op::Add;
      DEFAULT_SWITCH_CASE
    }
  };

  return createBinOp(mappingFunc, [this] { return factor(); }, Token::Type::MINUS, Token::Type::PLUS);
}

ast::Expr
Parser::factor()
{
  const auto mappingFunc = [](Token::Type lexerToken) {
    switch (lexerToken) {
      case Token::Type::SLASH: return BinOp::Op::Div;
      case Token::Type::STAR: return BinOp::Op::Mult;
      DEFAULT_SWITCH_CASE
    }
  };

  return createBinOp(mappingFunc, [this] { return unary(); }, Token::Type::SLASH, Token::Type::STAR);
}

ast::Expr
Parser::unary()
{
  if (auto op = match(Token::Type::BANG, Token::Type::MINUS)) {
    auto child = unary();

    UnaryOp::Op op2;
    switch (op->get().getType()) {
      case Token::Type::BANG: op2 = UnaryOp::Op::Nott;
      case Token::Type::MINUS: op2 = UnaryOp::Op::Negate;
      DEFAULT_SWITCH_CASE
    }

    return std::make_unique<
      UnaryOp,
      UnaryOp::Op,
      ast::Expr,
      size_t
    >(
      std::move(op2),
      std::move(child),
      Counter::next()
    );
  } else {
    return primary();
  }
}

ast::Expr
Parser::primary()
{
  if (auto op = match(Token::Type::NUM)) {
    auto numText = op->get().getContents();
    auto numDouble = std::stod(numText); // TODO: catch exceptions and throw my own ones
    return ast::num(std::move(numDouble));
  }
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

template <class... T>
const lexer::Token &
Parser::expect(T &&... tokenTypes)
{
  if (current_ + 1 >= tokens_.size() || peek(Token::Type::EOFF)) {
    std::stringstream message;
    message << "Unexpected end of file during parsing. Expected one of: ";
    ((message << std::forward<T>(tokenTypes) << ", "), ...);
    throw CompileError(current().getLineNumber(), ERROR_TAG, message.str(), "");
  }

  const auto &nextToken = tokens_[current_ + 1];
  // True if the next token matches any of the parameters.
  if (((nextToken.getType() == tokenTypes) || ...)) {
    return nextToken;
  }

  std::stringstream message;
  message << "Unexpected token ";
  message << nextToken;
  message << ". Expected one of: ";
  // I think this leaves a trailing comma on the end.
  // Not ideal but I'm not sure how else to do it.
  ((message << std::forward<T>(tokenTypes) << ", "), ...);
  throw CompileError(nextToken.getLineNumber(), ERROR_TAG, message.str(), "");
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

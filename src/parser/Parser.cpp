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
using ast::Expr;

// Ideally this would be more descriptive but it should not happen anyway...
#define DEFAULT_SWITCH_CASE default: throw std::runtime_error(std::string("Unexpected token type in mapping function. Line ") + std::to_string(__LINE__));

namespace {

constexpr auto ERROR_TAG = "Parser";

}

namespace parser {

Expr
Parser::parse(std::vector<Token> tokens)
{
  current_ = -1;
  tokens_ = std::move(tokens);
 
  // Exceptions flow out of here if parsing fails. Once we add statements, there will be
  // some kind of error recovery & error accumulation here, like in the lexer.
  auto expr = expression();

  // Expect that we are pointing at the last non-eof token, and the next (and last)
  // token is eof.
  const auto isParsingSuccessful =
    current_ == tokens_.size() - 2 && tokens_[tokens_.size() - 1].getType() == Token::Type::EOFF;
  if (!isParsingSuccessful) {
    std::optional<Token> currentOrNull = current_ < tokens_.size() ? std::optional(current()) : std::nullopt;
    throw CompileError(
      ERROR_TAG,
      "Expected end of program but there were more tokens remaining.",
      std::nullopt // TODO: pass SourceReference
    );
  }

  return expr;
}

Expr
Parser::expression()
{
  return equality();
}

Expr
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

Expr
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

Expr
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

Expr
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

Expr
Parser::unary()
{
  if (auto op = match(Token::Type::BANG, Token::Type::MINUS)) {
    auto child = unary();

    UnaryOp::Op op2;
    switch (op->get().getType()) {
      case Token::Type::BANG: op2 = UnaryOp::Op::Nott; break;
      case Token::Type::MINUS: op2 = UnaryOp::Op::Negate; break;
      DEFAULT_SWITCH_CASE
    }

    return std::make_unique<
      UnaryOp,
      UnaryOp::Op,
      Expr,
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

Expr
Parser::primary()
{
  if (auto op = match(Token::Type::NUM)) {
    auto numText = op->get().getContents();
    auto numDouble = textToDouble(numText);
    return ast::num(std::move(numDouble));
  } else if (auto op = match(Token::Type::STR)) {
    // Need to copy this string instead of moving in case we have to enter panic mode
    // and undo this part of the parsing. The string's contents would need
    // to be available when we come around again.
    // It would have been better to just store references to the part of the
    // source file containing the string and not try to carry around the
    // contents of the string.
    auto string = op->get().getContents();
    return ast::string(std::move(string));
  } else if (auto op = match(Token::Type::TRUE)) {
    return ast::truee();
  } else if (auto op = match(Token::Type::FALSE)) {
    return ast::falsee();
  } else if (auto op = match(Token::Type::NIL)) {
    return ast::nil();
  } else if (auto op = match(Token::Type::LPEREN)) {
    auto child = expression();
    expect(Token::Type::RPEREN);
    return ast::grouping(std::move(child));
  } else {
    // Since none of the above cases matched, this should fail with a nice
    // error message.
    expect(
      Token::Type::NUM, Token::Type::STR, Token::Type::TRUE, Token::Type::FALSE, Token::Type::NIL, Token::Type::LPEREN
    );
    // Just to make it compile -- the call above should throw anyway.
    // Would be better to make a helper that constructs the appropriate error message and throws it,
    // instead of relying on the call above. Can use `[[noreturn]]` to indicate that a function always throws.
    throw std::runtime_error("Unexpected token.");
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
    throw CompileError(ERROR_TAG, message.str(), std::nullopt /*TODO: pass SourceReference*/);
  }

  const auto &nextToken = tokens_[current_ + 1];
  // True if the next token matches any of the parameters.
  if (((nextToken.getType() == tokenTypes) || ...)) {
    advance();
    return nextToken;
  }

  std::stringstream message;
  message << "Unexpected token ";
  message << nextToken;
  message << ". Expected one of: ";
  // I think this leaves a trailing comma on the end.
  // Not ideal but I'm not sure how else to do it.
  ((message << std::forward<T>(tokenTypes) << ", "), ...);
  throw CompileError(ERROR_TAG, message.str(), std::nullopt /*TODO: pass SourceReference*/);
}

template <class BinOpMapFunc, class SubExprFunc, class... Ts>
Expr
Parser::createBinOp(const BinOpMapFunc &map, const SubExprFunc &subExpr, Ts &&... tokenTypes)
{
  Expr lhs = subExpr();
  while (auto op = match(std::forward<Ts>(tokenTypes)...)) {
    Expr rhs = subExpr();
    ast::BinOp::Op op2 = map(op->get().getType());
    // Unfortunately we can't use the factory functions easily here because a different
    // function needs to be called depending on the op of the lexer token, which we must
    // map to a BinOp op.
    // This could have been avoided by also adding factory functions that taken in the
    // BinOp::Op argument to decide which type of BinOp to make (then the existing factory
    // functions could call those).
    lhs = std::make_unique<
      ast::BinOp,
      Expr,
      ast::BinOp::Op,
      Expr,
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

double
Parser::textToDouble(const std::string &text)
{
  try {
    return std::stod(text);
  } catch (const std::invalid_argument &e) {
    //const auto &currentToken = current();
    std::stringstream stream;
    stream << "Unable to parse number into double-precision floating point. ";
    stream << "Internal error: " << e.what();
    throw CompileError(ERROR_TAG, stream.str(), std::nullopt /*TODO: pass SourceReference*/);
  } catch (const std::out_of_range &e) {
    //const auto &currentToken = current();
    constexpr auto message = "Number is out of range of double-precision floating point, so cannot be represented.";
    throw CompileError(ERROR_TAG, message, std::nullopt /*TODO: pass SourceReference*/);
  }
}

}

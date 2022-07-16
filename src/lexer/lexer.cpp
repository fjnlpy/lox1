#include "lexer/lexer.h"

#include <utility>
#include <stdexcept>
#include <cassert>

#include "utils/error.hpp"

namespace {

bool
isWhitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool
isEof(int peekResult)
{
  return peekResult == std::char_traits<char>::eof();
}
}

namespace lexer {

/// Token

Token::Token(Type tokenType, unsigned lineNumber, std::string contents)
  : tokenType_(tokenType)
  , contents_(std::move(contents))
  , lineNumber_(lineNumber)
  { }

/// Lexer

Lexer::Lexer() =default;

std::vector<Token>
Lexer::lex(const std::string &sourceCode)
{
  sourceCode_ = std::stringstream(sourceCode);
  // Reset state from last call (if any).
  tokens_.clear();
  currentLine_ = 1;
  currentLex_.clear();
  errors_.clear();

  while (sourceCode_) {
    char c;
    sourceCode_ >> c;
    currentLex_.push_back(c);
    lex(c);
  }

  if (!sourceCode_.eof()) {
    // Must have had an error with the stream. Can't do any more lexing, so fail.
    throw std::runtime_error("Unable to read from source code input stream.");
  }

  if (!errors_.empty()) {
    // Found one or more compilation errors. Fail with syntax error information
    // for each error we encountered.
    throw ErrorCollection(std::move(errors_));
  }

  addToken(Token::Type::EOFF);
  // TODO: I think move is correct here because tokens_ isn't a local var -- check effective cpp book.
  return std::move(tokens_);
}

void
Lexer::lex(char c)
{
  // Whitespace.
  if (isWhitespace(c)) {
    if (c == '\n') {
      ++currentLine_;
    }
    // Ignore it.
    return;
  }

  // Single-character tokens.
  switch (c) {
    case '(': addToken(Token::Type::LPEREN); return;
    case ')': addToken(Token::Type::RPEREN); return;
    case '{': addToken(Token::Type::LBRACE); return;
    case '}': addToken(Token::Type::RBRACE); return;
    case ',': addToken(Token::Type::COMMA); return;
    case '.': addToken(Token::Type::DOT); return;
    case '-': addToken(Token::Type::MINUS); return;
    case '+': addToken(Token::Type::PLUS); return;
    case ';': addToken(Token::Type::SEMICOLON); return;
    case '*': addToken(Token::Type::STAR); return;
  }

  // Single- or multi-character tokens.
  switch (c) {
    case '!': addToken(match('=') ? Token::Type::BANG_EQ : Token::Type::BANG); return;
    case '=': addToken(match('=') ? Token::Type::EQ_EQ : Token::Type::EQ); return;
    case '<': addToken(match('=') ? Token::Type::LT_EQ : Token::Type::LT); return;
    case '>': addToken(match('=') ? Token::Type::GT_EQ : Token::Type::GT); return;
  }

  // Comments
  if (c == '/') {
    if (match('/')) lexComment();
    else addToken(Token::Type::SLASH);
    return;
  }

  // Strings
  if (c == '"') {
    lexString();
    return;
  }

}

void
Lexer::addToken(Token::Type tokenType, bool includeContents = false)
{
  // TODO: should you always use emplace instead of move?
  tokens_.push_back(Token(tokenType, currentLine_, includeContents ? std::move(currentLex_) : ""));
  currentLex_.clear();
}

bool
Lexer::match(char d)
{
  assert(("Trying to match newline, which won't increment line counter", d != '\n'));
  return match(
    [d](char c) { return c == d; }
  );
}

bool
Lexer::match(const std::function<bool(char)> &predicate)
{
  char c2 = sourceCode_.peek();
  if (isEof(c2)) {
    return false;
  } else if (predicate(c2)) {
    // Extract c2 so it is consumed from the stream.
    sourceCode_.get();
    return true;
  } else {
    // If the character doesn't match, the stream is unchanged.
    return false;
  }
}

void
Lexer::lexComment()
{
  assert(("Should only be called when '//' of comment has been lexed", currentLex_ == "//"));

  int next = sourceCode_.peek();
  // Keep discarding characters until the line ends.
  // Don't grab newlines because we only want to handle
  // them in one place in the lexer.
  while (!isEof(next) && next != '\n') {
    sourceCode_.get();
  }
}

void
Lexer::lexString()
{
  assert(("should only be called when '\"' has been lexed", currentLex_ == "\""));

  int next;
  for (
    next = sourceCode_.peek();
    !isEof(next) && next != '"';
    next = sourceCode_.peek()
  ) {
    currentLex_.push_back(sourceCode_.get());
  }
  assert(isEof(next) || next == '"');

  if (isEof(next)) {
    // Note down this error and let the lexing continue. It will
    // fail straight away and report the error along with any others
    // from earlier.
    errors_.push_back(
      CompileError(
        currentLine_,
        "Lexer",
        "Unterminated string at end of file",
        currentLex_
      )
    );
  } else {
    assert(sourceCode_.get() == '"');

    // Make the token. It currently starts with a '"'; we
    // don't want that in the input so we have to remove it.
    currentLex_.erase(currentLex_.begin());
    addToken(Token::Type::STR, true);
  }
}

}

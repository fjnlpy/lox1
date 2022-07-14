#include "lexer/lexer.h"

#include <utility>
#include <stdexcept>
#include <cassert>

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

  while (sourceCode_) {
    char c;
    sourceCode_ >> c;
    currentLex_.push_back(c);
    lex(c);
  }

  if (!sourceCode_.eof()) {
    // Must have had an error with the stream. Can't do any more lexing, so fail.
     // TODO: more info?
     // TODO: have a utils::compiler_error instead?
    throw std::runtime_error("Unable to read from source code input stream.");
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
  char c2 = sourceCode_.peek();
  if (isEof(c2)) {
    return false;
  } else if (c2 == d) {
    // We know this is the same as c2, but extract it
    // from the stream so we don't see it again.
    sourceCode_.get();
    return true;
  } else {
    // If the character doesn't match, the stream is unchanged.
    return false;
  }
}

}

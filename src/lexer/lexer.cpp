#include "lexer/lexer.h"

#include <utility>
#include <stdexcept>

namespace lexer {

/// Token

Token::Token(Type tokenType, unsigned lineNumber)
  : Token(tokenType, lineNumber, "")
  { }

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

  while (sourceCode_) {
    // TODO: do lexing
  }

  if (!sourceCode_.eof()) {
    // Must have had an error with the stream. Can't do any more lexing, so fail.
     // TODO: more info?
     // TODO: have a utils::compiler_error instead?
    throw std::runtime_error("Unable to read from source code input stream.");
  }

  // TODO: should you always use emplace instead of move?
  tokens_.push_back(Token(Token::Type::EOFF, currentLine_));
  // TODO: I think move is correct here because tokens_ isn't a local var -- check effective cpp book.
  return std::move(tokens_);
}

}

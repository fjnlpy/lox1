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

}

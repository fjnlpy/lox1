#include "lexer/Lexer.h"

#include <utility>
#include <stdexcept>
#include <unordered_map>

#include "utils/Error.hpp"
#include "utils/Assert.hpp"

namespace {

constexpr auto ERROR_TAG = "Lexer";

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

bool
isNumber(char c)
{
  return c >= '0' && c <= '9';
}

bool
isIdentifierChar(char c)
{
  return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

}

namespace lexer {

/// Token

Token::Token(Type tokenType, unsigned lineNumber, std::string contents)
  : type_(tokenType)
  , contents_(std::move(contents))
  , lineNumber_(lineNumber)
  { }

const std::string &
Token::getContents() const
{
  return contents_;
}

Token::Type
Token::getType() const
{
  return type_;
}

unsigned
Token::getLineNumber() const
{
  return lineNumber_;
}

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

  // Extract using `get` instead of streaming, because the format is for "unformatted"
  // extraction e.g. doesn't strip whitespace (we don't want that because we're going
  // to do it ourselves).
  for (char c; sourceCode_.get(c); ) {
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
    currentLex_.clear();
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

  // Numbers: we allow integers and decimals, but no leading or trailing decimal points.
  if (isNumber(c)) {
    lexNumber();
    return;
  }

  // Identifiers and reserved words
  if (isIdentifierChar(c)) {
    lexIdentifierOrReservedWord();
    return;
  }

  // Unrecognised character.
  // TODO: better source snippet for this.
  errors_.push_back(
    CompileError(
      currentLine_,
      ERROR_TAG,
      std::string("Unrecognized character: '") + c + "'; ASCII: " + std::to_string(static_cast<int>(c)),
      ""
    )
  );
}

void
Lexer::addToken(Token::Type tokenType, bool includeContents)
{
  // TODO: should you always use emplace instead of move?
  tokens_.push_back(Token(tokenType, currentLine_, includeContents ? std::move(currentLex_) : ""));
  currentLex_.clear();
}

bool
Lexer::match(char d)
{
  ASSERT(d != '\n' && "Trying to match newline, which won't increment line counter");
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
    consume();
    return true;
  } else {
    // If the character doesn't match, the stream is unchanged.
    return false;
  }
}

int
Lexer::peekNext()
{
  // We need a lookahead of two for lexing numbers but c++'s streams don't
  // have that functionality. Instead, read two characters from the stream and
  // put them back afterwards.
  const int c1 = sourceCode_.peek();
  if (isEof(c1)) {
    return c1;
  }

  const char c11 = sourceCode_.get(); // progress the stream
  ASSERT(c11 == c1);

  const int c2 = sourceCode_.peek();
  sourceCode_.unget();
  return c2;

}

void
Lexer::lexComment()
{
  ASSERT(currentLex_ == "//" && "Should only be called when '//' of comment has been lexed");

  // Keep discarding characters until the line ends.
  // Don't grab newlines because we only want to handle
  // them in one place in the lexer.
  for ( int next = sourceCode_.peek();
        !isEof(next) && next != '\n';
        next = sourceCode_.peek()
  ) {
    sourceCode_.get(); // don't add to current lex since we don't want to keep it
  }

  // Discard all the characters in the comment. They are not useful to the compiler.
  currentLex_.clear();
}

void
Lexer::lexString()
{
  ASSERT(currentLex_ == "\"" && "should only be called when '\"' has been lexed");

  int next;
  for (
    next = sourceCode_.peek();
    !isEof(next) && next != '"';
    next = sourceCode_.peek()
  ) {
    consume();
  }
  ASSERT(isEof(next) || next == '"');

  if (isEof(next)) {
    // Note down this error and let the lexing continue. It will
    // fail straight away and report the error along with any others
    // from earlier.
    errors_.push_back(
      CompileError(
        currentLine_,
        ERROR_TAG,
        "Unterminated string at end of file",
        currentLex_
      )
    );
  } else {
    const char c = sourceCode_.get(); // discard the '"'
    ASSERT(c == '"');

    // Make the token. It currently starts with a '"'; we
    // don't want that in the input so we have to remove it.
    currentLex_.erase(currentLex_.begin());
    addToken(Token::Type::STR, true);
  }
}

void
Lexer::lexNumber()
{
  while (match(isNumber)) {
    // Keep collecting the numbers.
  }

  // We support decimal points but only if they're followed by more numbers.
  // e.g. 2.3 is allowed but 2. is not.
  if (sourceCode_.peek() == '.' && isNumber(peekNext())) {
    consume(); // take the decimal point
    while (match(isNumber)) {
      // Keep collecting the numbers.
    }
  }

  addToken(Token::Type::NUM, true);
}

void
Lexer::lexIdentifierOrReservedWord()
{
  // We have already consumed the first character. Subsequent ones can either be indentifier
  // characters or numbers.
  while (match([](char c) { return isIdentifierChar(c) || isNumber(c); })) {
    // Keep consuming.
  }

  // We have consumed the entire identifier. Check if it's a reserved word.
  static const std::unordered_map<std::string, Token::Type> reservedWordMap {
    { "for", Token::Type::FOR },
    { "if", Token::Type::IF },
    { "and", Token::Type::AND },
    { "class", Token::Type::CLASS },
    { "else", Token::Type::ELSE },
    { "false", Token::Type::FALSE },
    { "fun", Token::Type::FUN },
    { "nil", Token::Type::NIL },
    { "or", Token::Type::OR },
    { "print", Token::Type::PRINT },
    { "return", Token::Type::RETURN },
    { "super", Token::Type::SUPER },
    { "this", Token::Type::THIS },
    { "true", Token::Type::TRUE },
    { "var", Token::Type::VAR },
    { "while", Token::Type::WHILE },
  };

  const auto reservedWordTokenIt = reservedWordMap.find(currentLex_.c_str());
  if (reservedWordTokenIt == reservedWordMap.cend()) {
    addToken(Token::Type::ID, true);
  } else {
    addToken(reservedWordTokenIt->second, false);
  }
}

void
Lexer::consume()
{
  currentLex_.push_back(sourceCode_.get());
}

}

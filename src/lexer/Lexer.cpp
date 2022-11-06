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

void
printContents(std::ostream &os, const std::string &contents, bool useDoubleQuotes)
{
  os << "(";
  if (useDoubleQuotes) {
    os << "\"";
  } else {
    os << "'";
  }

  if (contents.size() > 10) {
    // Long string. Just show the start and end.
    os << contents.substr(0, 5);
    os << "[...]";
    os << contents.substr(contents.size() - 6, 5);
  } else {
    os << contents;
  }

  if (useDoubleQuotes) {
    os << "\"";
  } else {
    os << "'";
  }
  os << ")";
}

}

namespace lexer {

/// Token

Token::Token(Type tokenType, std::string contents, std::unique_ptr<const SourceReference> sourceReference)
  : type_(tokenType)
  , contents_(std::move(contents))
  , sourceReference_(std::move(sourceReference))
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

const SourceReference *
Token::getSourceReference() const
{
  return sourceReference_.get();
}

// These methods basically invert what the lexer does. It's kind of
// silly in a way because when lexing we could just store the entire
// string corrresponding to each lex, then we could get the contents of that
// rather than having to do this. But that would be a waste of memory for
// almost all token types. Probably the best way to do it would be to
// make token effectively a range onto the input program (by storing
// starting and ending line & column for each token), which we
// could then go and re-read to recover the input. I think that's what
// proper compilers do but I didn't think it was necessary at the time...
// Edit: this actually exists now. I added it because it's useful for
// error reporting. If I had designed it in from then start then
// probably this stuff wouldn't have been necessary. Now that's it's
// here, is it worth removing this?

std::ostream &
operator<<(std::ostream &os, const Token &token)
{
  // This method just forwards to ostream for the token type,
  // but also adds some metadata in cases where we don't know
  // based on the token type alone what the user typed (corresponds
  // pretty well with the cases where we capture the contents of
  // the lex inside the token, e.g. identifiers, numbers).
  os << token.getType();
  switch (token.getType()) {
    case Token::Type::ID:
    case Token::Type::NUM:
      printContents(os, token.getContents(), false); break;
    case Token::Type::STR:
      printContents(os, token.getContents(), true); break;

    // For other cases, string representation of token type itself
    // should be sufficient.
    default: break;
  }

  return os;
}

std::ostream &
operator<<(std::ostream &os, const Token::Type &tokenType)
{
  switch (tokenType) {
    case Token::Type::LPEREN:    os << "LPEREN"; break;
    case Token::Type::RPEREN:    os << "RPEREN"; break;
    case Token::Type::LBRACE:    os << "LBRACE"; break;
    case Token::Type::RBRACE:    os << "RBRACE"; break;
    case Token::Type::COMMA:     os << "COMMA"; break;
    case Token::Type::DOT:       os << "DOT"; break;
    case Token::Type::MINUS:     os << "MINUS"; break;
    case Token::Type::PLUS:      os << "PLUS"; break;
    case Token::Type::SEMICOLON: os << "SEMICOLON"; break;
    case Token::Type::SLASH:     os << "SLASH"; break;
    case Token::Type::STAR:      os << "STAR"; break;
    case Token::Type::BANG:      os << "BANG"; break;
    case Token::Type::BANG_EQ:   os << "BANG_EQ"; break;
    case Token::Type::EQ:        os << "EQ"; break;
    case Token::Type::EQ_EQ:     os << "EQ_EQ"; break;
    case Token::Type::GT:        os << "GT"; break;
    case Token::Type::GT_EQ:     os << "GT_EQ"; break;
    case Token::Type::LT:        os << "LT"; break;
    case Token::Type::LT_EQ:     os << "LT_EQ"; break;
    case Token::Type::AND:       os << "AND"; break;
    case Token::Type::CLASS:     os << "CLASS"; break;
    case Token::Type::ELSE:      os << "ELSE"; break;
    case Token::Type::TRUE:      os << "TRUE"; break;
    case Token::Type::FALSE:     os << "FALSE"; break;
    case Token::Type::FUN:       os << "FUN"; break;
    case Token::Type::FOR:       os << "FOR"; break;
    case Token::Type::IF:        os << "IF"; break;
    case Token::Type::NIL:       os << "NIL"; break;
    case Token::Type::OR:        os << "OR"; break;
    case Token::Type::PRINT:     os << "PRINT"; break;
    case Token::Type::RETURN:    os << "RETURN"; break;
    case Token::Type::SUPER:     os << "SUPER"; break;
    case Token::Type::THIS:      os << "THIS"; break;
    case Token::Type::VAR:       os << "VAR"; break;
    case Token::Type::WHILE:     os << "WHILE"; break;
    case Token::Type::ID:        os << "ID"; break;
    case Token::Type::STR:       os << "STR"; break;
    case Token::Type::NUM:       os << "NUM"; break;
    case Token::Type::EOFF:      os << "EOF"; break;
  }

  switch (tokenType) {
    case Token::Type::LPEREN:    os << "('(')"; break;
    case Token::Type::RPEREN:    os << "(')')"; break;
    case Token::Type::LBRACE:    os << "('{')"; break;
    case Token::Type::RBRACE:    os << "('}')"; break;
    case Token::Type::COMMA:     os << "(',')"; break;
    case Token::Type::DOT:       os << "('.')"; break;
    case Token::Type::MINUS:     os << "('-')"; break;
    case Token::Type::PLUS:      os << "('+')"; break;
    case Token::Type::SEMICOLON: os << "(';')"; break;
    case Token::Type::SLASH:     os << "('/')"; break;
    case Token::Type::STAR:      os << "('*')"; break;
    case Token::Type::BANG:      os << "('!')"; break;
    case Token::Type::BANG_EQ:   os << "('!')"; break;
    case Token::Type::EQ:        os << "('=')"; break;
    case Token::Type::EQ_EQ:     os << "('=')"; break;
    case Token::Type::GT:        os << "('>')"; break;
    case Token::Type::GT_EQ:     os << "('>')"; break;
    case Token::Type::LT:        os << "('<')"; break;
    case Token::Type::LT_EQ:     os << "('<')"; break;
    case Token::Type::AND:       os << "('and')"; break;
    case Token::Type::CLASS:     os << "('class')"; break;
    case Token::Type::ELSE:      os << "('else')"; break;
    case Token::Type::TRUE:      os << "('true')"; break;
    case Token::Type::FALSE:     os << "('false')"; break;
    case Token::Type::FUN:       os << "('fun')"; break;
    case Token::Type::FOR:       os << "('for')"; break;
    case Token::Type::IF:        os << "('if')"; break;
    case Token::Type::NIL:       os << "('nil')"; break;
    case Token::Type::OR:        os << "('or')"; break;
    case Token::Type::PRINT:     os << "('print')"; break;
    case Token::Type::RETURN:    os << "('return')"; break;
    case Token::Type::SUPER:     os << "('super')"; break;
    case Token::Type::THIS:      os << "('this')"; break;
    case Token::Type::VAR:       os << "('var')"; break;
    case Token::Type::WHILE:     os << "('while')"; break;
    // For the other cases, either there is no literal string
    // representation (EOF) or there are multiple (identifiers,
    // strings, etc.). If you want to print one of these tokens,
    // use the token ostream operator instead, which will also
    // output the captured contents for that token.
    default: break;
  }

  return os;
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
  currentColumn_ = -1;
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
  ++currentColumn_;

  // Whitespace.
  if (isWhitespace(c)) {
    if (c == '\n') {
      ++currentLine_;
      currentColumn_ = -1;
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
  errors_.push_back(
    CompileError(
      ERROR_TAG,
      std::string("Unrecognized character: '") + c + "'; ASCII: " + std::to_string(static_cast<int>(c)),
      std::make_unique<SingleLineReference>(currentLine_, currentColumn_, currentColumn_)
    )
  );
}

void
Lexer::addToken(Token::Type tokenType, bool includeContents)
{
  tokens_.push_back(
    Token(
      tokenType,
      includeContents ? std::move(currentLex_) : "",
      std::make_unique<SingleLineReference>(currentLine_, currentColumn_, currentColumn_ + currentLex_.size())
    )
  );
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
        ERROR_TAG,
        "Unterminated string at end of file",
        std::make_unique<SingleLineReference>(currentLine_, currentColumn_, currentColumn_ + currentLex_.size())
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

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <ostream>
#include <functional>

#include "utils/Error.hpp"

namespace lexer {

class Token {
public:
  enum class Type {
    LPEREN, RPEREN, LBRACE, RBRACE, COMMA, DOT, MINUS,
    PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQ, EQ, EQ_EQ, GT, GT_EQ, LT, LT_EQ,

    ID, STR, NUM,

    AND, CLASS, ELSE, TRUE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, VAR, WHILE,

    EOFF, // "EOF" is already used, as a macro.

    START
  };

  Token(Type, std::string, std::optional<const SourceReference>);

  Type getType() const;
  const std::string &getContents() const;
  std::optional<SourceReference> getSourceReference() const;

  friend std::ostream& operator<<(std::ostream&, const Token &);
  friend std::ostream& operator<<(std::ostream&, const Token::Type &);

private:
  Type type_;
  // The contents don't actually need to be stored here as long as whenever `getContents`
  // is called the original program text is available: then we could look up the source reference
  // in the program text (although what if the source reference hasn't been provided?).
  std::string contents_;
  std::optional<const SourceReference> sourceReference_;

};

class Lexer {
public:

  Lexer();

  std::vector<Token> lex(const std::string &sourceCode);

private:
  std::stringstream sourceCode_;
  std::vector<Token> tokens_;
  unsigned currentLine_ = 1;
  unsigned currentColumn_ = 0;
  std::string currentLex_;
  std::vector<CompileError> errors_;

  void lex(char c);
  void addToken(Token::Type tokenType, bool includeContents = false);
  void clearCurrentLex();
  bool match(char d);
  bool match(const std::function<bool(char)> &predicate);
  int peekNext();
  void lexComment();
  void lexString();
  void lexNumber();
  void lexIdentifierOrReservedWord();
  void consume();

};

}

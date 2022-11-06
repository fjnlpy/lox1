#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <ostream>
#include <functional>
#include <memory>

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

    EOFF // "EOF" is already used, as a macro.
  };

  Token(Type tokenType, std::string contents, std::unique_ptr<const SourceReference> sourceReference);

  Type getType() const;
  const std::string &getContents() const;
  const SourceReference *getSourceReference() const;

  friend std::ostream& operator<<(std::ostream&, const Token &);
  friend std::ostream& operator<<(std::ostream&, const Token::Type &);

private:
  const Type type_;
  const std::string contents_;
  const std::unique_ptr<const SourceReference> sourceReference_

};

class Lexer {
public:

  Lexer();

  std::vector<Token> lex(const std::string &sourceCode);

private:
  std::stringstream sourceCode_;
  std::vector<Token> tokens_;
  unsigned currentLine_ = 1;
  std::string currentLex_;
  std::vector<CompileError> errors_;

  void lex(char c);
  void addToken(Token::Type tokenType, bool includeContents = false);
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

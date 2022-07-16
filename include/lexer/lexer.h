#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <functional>

#include "utils/error.hpp"

namespace lexer {

class Token {
public:
  enum class Type {
    LPEREN, RPEREN, LBRACE, RBRACE, COMMA, DOT, MINUS,
    PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQ, EQ, EQ_EQ, GT, GT_EQ, LT, LT_EQ,

    ID, STR, NUM,

    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, VAR, WHILE,

    EOFF // "EOF" is already used, as a macro.
  };

  Token(Type tokenType, unsigned lineNunber, std::string contents);

private:
  Type tokenType_;
  std::string contents_;
  unsigned lineNumber_;

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
  void addToken(Token::Type tokenType, bool includeContents);
  bool match(char d);
  bool match(const std::function<bool(char)> &predicate);
  int peekNext();
  void lexComment();
  void lexString();
  void lexNumber();

};

}

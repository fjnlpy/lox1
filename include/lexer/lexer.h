#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace lexer {

class Token {
public:
  enum Type {
    LPEREN, RPEREN, LBRACE, RBRACE, COMMA, DOT, MINUS,
    PLUS, SEMICOLON, SLASH, STAR,

    BANG, BANG_EQ, EQ, EQ_EQ, GT, GT_EQ, LT, LT_EQ,

    ID, STR, NUM,

    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, VAR, WHILE,

    EOFF // "EOF" is already used, as a macro.
  };

  Token(Type tokenType, unsigned lineNumber);

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

};

}

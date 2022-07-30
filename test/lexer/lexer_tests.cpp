#include <gtest/gtest.h>

#include <vector>

#include "lexer/lexer.h"
#include "utils/logging.hpp"

using namespace lexer;

namespace {

#define EXPECT_TOKEN_TYPE(expectedType, token) EXPECT_EQ(expectedType, token.getType())

#define EXPECT_EOF(token) ASSERT_TRUE(token.size() > 0); EXPECT_TOKEN_TYPE(Token::Type::EOFF, (*(tokens.cend() - 1)))

}

TEST(LexerTests, TestEmptyInput) {
  Lexer lexer;

  std::vector<Token> tokens;
  try {
    tokens = lexer.lex("");
  } catch (const ErrorCollection &e) {
    FAIL() << e.what();
  }

  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(Token::Type::EOFF, tokens[0].getType());
}

TEST(LexerTests, TestEveryCharacter) {
  Lexer lexer;

  const auto input =
R"(
(){},.-+;/*!!== ==> >=< <=abc"I am str <><>" 22
and class else true false fun for if nil or print return
super this var while

.)";

  std::vector<Token> tokens;
  try {
    tokens = lexer.lex(input);
  } catch (const ErrorCollection &e) {
    FAIL() << e.what();
  }

  ASSERT_EQ(40, tokens.size());
  EXPECT_TOKEN_TYPE(Token::Type::LPEREN, tokens[0]);
  EXPECT_TOKEN_TYPE(Token::Type::RPEREN, tokens[1]);
  EXPECT_TOKEN_TYPE(Token::Type::LBRACE, tokens[2]);
  EXPECT_TOKEN_TYPE(Token::Type::RBRACE, tokens[3]);

  EXPECT_TOKEN_TYPE(Token::Type::COMMA, tokens[4]);
  EXPECT_TOKEN_TYPE(Token::Type::DOT, tokens[5]);
  EXPECT_TOKEN_TYPE(Token::Type::MINUS, tokens[6]);
  EXPECT_TOKEN_TYPE(Token::Type::PLUS, tokens[7]);

  EXPECT_TOKEN_TYPE(Token::Type::SEMICOLON, tokens[8]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[9]);
  EXPECT_TOKEN_TYPE(Token::Type::STAR, tokens[10]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[11]);

  EXPECT_TOKEN_TYPE(Token::Type::BANG_EQ, tokens[12]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ, tokens[13]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ_EQ, tokens[14]);

  EXPECT_TOKEN_TYPE(Token::Type::GT, tokens[15]);
  EXPECT_TOKEN_TYPE(Token::Type::GT_EQ, tokens[16]);
  EXPECT_TOKEN_TYPE(Token::Type::LT, tokens[17]);
  EXPECT_TOKEN_TYPE(Token::Type::LT_EQ, tokens[18]);

  EXPECT_TOKEN_TYPE(Token::Type::ID, tokens[19]);
  EXPECT_EQ("abc", tokens[19].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::STR, tokens[20]);
  EXPECT_EQ("I am str <><>", tokens[20].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[21]);
  EXPECT_EQ("22", tokens[21].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::AND, tokens[22]);
  EXPECT_TOKEN_TYPE(Token::Type::CLASS, tokens[23]);
  EXPECT_TOKEN_TYPE(Token::Type::ELSE, tokens[24]);
  EXPECT_TOKEN_TYPE(Token::Type::TRUE, tokens[25]);
  EXPECT_TOKEN_TYPE(Token::Type::FALSE, tokens[26]);
  EXPECT_TOKEN_TYPE(Token::Type::FUN, tokens[27]);
  EXPECT_TOKEN_TYPE(Token::Type::FOR, tokens[28]);
  EXPECT_TOKEN_TYPE(Token::Type::IF, tokens[29]);
  EXPECT_TOKEN_TYPE(Token::Type::NIL, tokens[30]);

  EXPECT_TOKEN_TYPE(Token::Type::OR, tokens[31]);
  EXPECT_TOKEN_TYPE(Token::Type::PRINT, tokens[32]);
  EXPECT_TOKEN_TYPE(Token::Type::RETURN, tokens[33]);
  EXPECT_TOKEN_TYPE(Token::Type::SUPER, tokens[34]);
  EXPECT_TOKEN_TYPE(Token::Type::THIS, tokens[35]);
  EXPECT_TOKEN_TYPE(Token::Type::VAR, tokens[36]);
  EXPECT_TOKEN_TYPE(Token::Type::WHILE, tokens[37]);

  EXPECT_TOKEN_TYPE(Token::Type::DOT, tokens[38]);

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestLexNumber) {
  Lexer lexer;
  const auto input = "11";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(2, tokens.size());
  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[0]);
  EXPECT_EQ("11", tokens[0].getContents());
  EXPECT_EOF(tokens);
}

/* TODO:
    - lex everything once
    - unsupported characters (try putting ident after)
    - string stuff (and contents!)
    - number stuff (and contents!)
    - comments stuff
    - single tokens next to their multi token brothers
    - identifier and reserved word stuff (and contents!)
*/

#include <gtest/gtest.h>

#include <vector>

#include "lexer/lexer.h"
#include "utils/logging.hpp"

using namespace lexer;

namespace {

#define EXPECT_TOKEN_TYPE(expectedType, token) EXPECT_EQ(expectedType, token.getType())

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

  ASSERT_EQ(39, tokens.size()); //FIXME: should be 40?
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
  EXPECT_TOKEN_TYPE(Token::Type::STR, tokens[20]);


  FAIL() << tokens[20].getContents() << "; " << tokens[21].getContents() << "; " << tokens[22].getContents();

  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[21]);

  EXPECT_TOKEN_TYPE(Token::Type::AND, tokens[22]);
  EXPECT_TOKEN_TYPE(Token::Type::CLASS, tokens[23]);
  EXPECT_TOKEN_TYPE(Token::Type::ELSE, tokens[24]);

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

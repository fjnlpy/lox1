#include "gtest/gtest.h"

#include <variant>
#include <vector>

#include "parser/Parser.h"

using lexer::Token;

TEST(ParserTests, TestPrimaryExpression) {
  parser::Parser parser;

  auto expr = parser.parse({Token(Token::Type::STR, 1, "hello")});

  ASSERT_EQ("hello", std::get<ast::StringPtr>(expr)->value());
}

TEST(ParserTests, TestOperatorPrecedence) {
  parser::Parser parser;

  auto expr = parser.parse({
    Token(Token::Type::NUM, 1, "1"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "2"),
    Token(Token::Type::STAR, 1, ""),
    Token(Token::Type::NUM, 1, "3"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "4")
  });

  // TODO: make a helper for asserting same. Just run the pretty printer
  // on both (since it's deterministic and has unique output (except fp rounding)
  // for unique tree). Probably comment that it's not exactly ideal though,
  // but fine for our current purposes.
}

TEST(ParserTests, TestTrailingBinOp) {

}

TEST(ParserTests, TestGrouping) {

}

TEST(ParserTests, TestUnclosedGroup) {

}

TEST(ParserTests, TestNestedUnaryOps) {

}

TEST(ParserTests, TestLeadingBinOp) {

}

TEST(ParserTests, TestInvalidPrimaryExpression) {

}

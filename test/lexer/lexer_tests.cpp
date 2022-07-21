#include <gtest/gtest.h>

#include <vector>

#include "lexer/lexer.h"
#include "utils/logging.hpp"

using namespace lexer;

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

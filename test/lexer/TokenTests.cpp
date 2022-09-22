#include <gtest/gtest.h>

#include "lexer/Lexer.h"

using namespace lexer;

TEST(TokenTests, TestProperties) {
  const Token token(Token::Type::ID, 2, "abcd");

  EXPECT_EQ(token.getType(), Token::Type::ID);
  EXPECT_EQ(token.getLineNumber(), 2);
  EXPECT_EQ(token.getContents(), "abcd");
}

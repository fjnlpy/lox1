#include <gtest/gtest.h>

#include "lexer/Lexer.h"

using namespace lexer;

TEST(TokenTests, TestProperties) {
  const Token token(Token::Type::ID, "abcd", SourceReference(2));

  EXPECT_EQ(token.getType(), Token::Type::ID);
  EXPECT_EQ(token.getSourceReference().value().getLineNumber(), 2);
  EXPECT_EQ(token.getContents(), "abcd");
}

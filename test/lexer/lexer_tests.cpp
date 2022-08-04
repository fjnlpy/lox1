#include <gtest/gtest.h>

#include <vector>
#include <optional>

#include "lexer/lexer.h"
#include "utils/logging.hpp"

using namespace lexer;

namespace {

#define EXPECT_TOKEN_TYPE(expectedType, token) EXPECT_EQ(expectedType, token.getType())

#define EXPECT_EOF(token) ASSERT_TRUE(token.size() > 0); EXPECT_TOKEN_TYPE(Token::Type::EOFF, (*(tokens.cend() - 1)))

void
expectSingleStringLex(const std::string &string)
{
  Lexer lexer;
  const auto input = std::string("\"") + string + "\"";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(2, tokens.size());
  //LOGD(tokens[0].getContents());
  EXPECT_TOKEN_TYPE(Token::Type::STR, tokens[0]);
  EXPECT_EQ(string, tokens[0].getContents());
  EXPECT_EOF(tokens);
}

void
expectSingleNumberLex(const std::string &number)
{
  Lexer lexer;
  const auto tokens = lexer.lex(number);
  ASSERT_EQ(2, tokens.size());
  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[0]);
  EXPECT_EQ(number, tokens[0].getContents());
  EXPECT_EOF(tokens);
}

template <class T> // totally unnecessary :)
void
expectIdentifierToken(const Token &token, T &&contents, std::optional<unsigned> lineNumber = std::nullopt)
{
  EXPECT_EQ(Token::Type::ID, token.getType());
  EXPECT_EQ(std::forward<T>(contents), token.getContents());
  if (lineNumber) {
    EXPECT_EQ(*lineNumber, token.getLineNumber());
  }
}

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

TEST(LexerTests, TestUnsupportedCharacter) {
  Lexer lexer;
  const auto input = "@&^#:hello~#";

  try {
    const auto tokens = lexer.lex(input);
    FAIL() << "Expecting lexing to fail due to unrecognized characters.";
  } catch (const ErrorCollection &e) {
    ASSERT_EQ(7, e.errors().size());
    //LOGD(e.what()); // for manual testing
  }
}

TEST(LexerTests, TestEmptyString) {
  const auto string = "";
  expectSingleStringLex(string);
}

TEST(LexerTests, TestMultiLineString) {
  const auto string = "hello\nhello\n";
  expectSingleStringLex(string);
}

TEST(LexerTests, TestStringWithComment) {
  expectSingleStringLex("hello//commentbutnotcomment//notcomment");
}

TEST(LexerTests, TestNonTerminatedString) {
  Lexer lexer;
  const auto input = "\"not terminated";
  EXPECT_THROW(lexer.lex(input), ErrorCollection);
}

TEST(LexerTests, TestStringOfReservedThings) {
  expectSingleStringLex("and or super + - -+ <= 12345");
}

TEST(LexerTests, TestAdjacentStrings) {
  Lexer lexer;
  const auto input = "\"s1\"\"s2\"";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(3, tokens.size());
  EXPECT_EQ("s1", tokens[0].getContents());
  EXPECT_EQ("s2", tokens[1].getContents());
  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestLexWholeNumber) {
  expectSingleNumberLex("11");
}

TEST(LexerTests, TestLexDecimalNumber) {
  expectSingleNumberLex("2.25");
}

TEST(LexerTests, TestLexComment) {
  Lexer lexer;
  const auto input = "// I am a comment";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(1, tokens.size());
  EXPECT_EOF(tokens);
  ASSERT_EQ(1, tokens[0].getLineNumber());
}

TEST(LexerTests, TestLexCommentAndEndOfLine) {
  Lexer lexer;
  const auto input = "// I am a comment\n";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(1, tokens.size());
  EXPECT_EOF(tokens);
  ASSERT_EQ(2, tokens[0].getLineNumber())
    << "Due to newline, EOF should be on second line";
}

TEST(LexerTests, TestLexCommentWithManySlashes) {
  Lexer lexer;
  const auto input = "/// I // am / a comment //";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(1, tokens.size());
  EXPECT_EOF(tokens);
  ASSERT_EQ(1, tokens[0].getLineNumber());
}

TEST(LexerTests, TextSingleAndMultiCharTokens) {
  Lexer lexer;
  const auto input = "!!===!<==>=/ /";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(10, tokens.size());

  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[0]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG_EQ, tokens[1]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ_EQ, tokens[2]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[3]);
  EXPECT_TOKEN_TYPE(Token::Type::LT_EQ, tokens[4]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ, tokens[5]);
  EXPECT_TOKEN_TYPE(Token::Type::GT_EQ, tokens[6]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[7]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[8]);

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestIdentifiers) {
  Lexer lexer;
  const auto input = "abc abc123 123abc _1212 _a_b_c a1a";
  const auto tokens = lexer.lex(input);

  expectIdentifierToken(tokens[0], "abc");
  expectIdentifierToken(tokens[1], "abc123");
  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[2]);
  expectIdentifierToken(tokens[3], "abc");
  expectIdentifierToken(tokens[4], "_1212");
  expectIdentifierToken(tokens[5], "_a_b_c");
  expectIdentifierToken(tokens[6], "a1a");

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestIdentAndReservedWord) {
  Lexer lexer;
  const auto input = "formula andrew footprints foot print";
  const auto tokens = lexer.lex(input);

  expectIdentifierToken(tokens[0], "formula");
  expectIdentifierToken(tokens[1], "andrew");
  expectIdentifierToken(tokens[2], "footprints");
  expectIdentifierToken(tokens[3], "foot");
  EXPECT_TOKEN_TYPE(Token::Type::PRINT, tokens[4]);

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestLineNumbers) {
  Lexer lexer;
  const auto input = "abc\ndef ghi\n\njkl";
  const auto tokens = lexer.lex(input);

  expectIdentifierToken(tokens[0], "abc", 1);
  expectIdentifierToken(tokens[1], "def", 2);
  expectIdentifierToken(tokens[3], "jkl", 4);

  EXPECT_EOF(tokens);
}

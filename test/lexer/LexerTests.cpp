#include <gtest/gtest.h>

#include <vector>
#include <optional>

#include "lexer/Lexer.h"
#include "utils/Logging.hpp"

using namespace lexer;

namespace {

#define EXPECT_TOKEN_TYPE(expectedType, token) EXPECT_EQ(expectedType, token.getType())

#define EXPECT_EOF(tokens) ASSERT_TRUE(tokens.size() > 0); EXPECT_TOKEN_TYPE(Token::Type::EOFF, (*(tokens.cend() - 1)))

#define EXPECT_START(tokens) ASSERT_TRUE(tokens.size() > 0); EXPECT_TOKEN_TYPE(Token::Type::START, (*(tokens.cbegin())))

void
expectSingleStringLex(const std::string &string)
{
  Lexer lexer;
  const auto input = std::string("\"") + string + "\"";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(3, tokens.size());
  EXPECT_START(tokens);
  EXPECT_TOKEN_TYPE(Token::Type::STR, tokens[1]);
  EXPECT_EQ(string, tokens[1].getContents());
  EXPECT_EOF(tokens);
}

void
expectSingleNumberLex(const std::string &number)
{
  Lexer lexer;
  const auto tokens = lexer.lex(number);
  ASSERT_EQ(3, tokens.size());
  EXPECT_START(tokens);
  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[1]);
  EXPECT_EQ(number, tokens[1].getContents());
  EXPECT_EOF(tokens);
}

template <class T> // totally unnecessary :)
void
expectIdentifierToken(const Token &token, T &&contents, std::optional<unsigned> lineNumber = std::nullopt)
{
  EXPECT_EQ(Token::Type::ID, token.getType());
  EXPECT_EQ(std::forward<T>(contents), token.getContents());
  if (lineNumber) {
    EXPECT_EQ(*lineNumber, token.getSourceReference().value().getLineNumber());
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

  EXPECT_EQ(2, tokens.size());
  EXPECT_EQ(Token::Type::START, tokens[0].getType());
  EXPECT_EQ(Token::Type::EOFF, tokens[1].getType());
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

  ASSERT_EQ(41, tokens.size());
  EXPECT_START(tokens);
  EXPECT_TOKEN_TYPE(Token::Type::LPEREN, tokens[1]);
  EXPECT_TOKEN_TYPE(Token::Type::RPEREN, tokens[2]);
  EXPECT_TOKEN_TYPE(Token::Type::LBRACE, tokens[3]);
  EXPECT_TOKEN_TYPE(Token::Type::RBRACE, tokens[4]);

  EXPECT_TOKEN_TYPE(Token::Type::COMMA, tokens[5]);
  EXPECT_TOKEN_TYPE(Token::Type::DOT, tokens[6]);
  EXPECT_TOKEN_TYPE(Token::Type::MINUS, tokens[7]);
  EXPECT_TOKEN_TYPE(Token::Type::PLUS, tokens[8]);

  EXPECT_TOKEN_TYPE(Token::Type::SEMICOLON, tokens[9]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[10]);
  EXPECT_TOKEN_TYPE(Token::Type::STAR, tokens[11]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[12]);

  EXPECT_TOKEN_TYPE(Token::Type::BANG_EQ, tokens[13]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ, tokens[14]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ_EQ, tokens[15]);

  EXPECT_TOKEN_TYPE(Token::Type::GT, tokens[16]);
  EXPECT_TOKEN_TYPE(Token::Type::GT_EQ, tokens[17]);
  EXPECT_TOKEN_TYPE(Token::Type::LT, tokens[18]);
  EXPECT_TOKEN_TYPE(Token::Type::LT_EQ, tokens[19]);

  EXPECT_TOKEN_TYPE(Token::Type::ID, tokens[20]);
  EXPECT_EQ("abc", tokens[20].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::STR, tokens[21]);
  EXPECT_EQ("I am str <><>", tokens[21].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[22]);
  EXPECT_EQ("22", tokens[22].getContents());

  EXPECT_TOKEN_TYPE(Token::Type::AND, tokens[23]);
  EXPECT_TOKEN_TYPE(Token::Type::CLASS, tokens[24]);
  EXPECT_TOKEN_TYPE(Token::Type::ELSE, tokens[25]);
  EXPECT_TOKEN_TYPE(Token::Type::TRUE, tokens[26]);
  EXPECT_TOKEN_TYPE(Token::Type::FALSE, tokens[27]);
  EXPECT_TOKEN_TYPE(Token::Type::FUN, tokens[28]);
  EXPECT_TOKEN_TYPE(Token::Type::FOR, tokens[29]);
  EXPECT_TOKEN_TYPE(Token::Type::IF, tokens[30]);
  EXPECT_TOKEN_TYPE(Token::Type::NIL, tokens[31]);

  EXPECT_TOKEN_TYPE(Token::Type::OR, tokens[32]);
  EXPECT_TOKEN_TYPE(Token::Type::PRINT, tokens[33]);
  EXPECT_TOKEN_TYPE(Token::Type::RETURN, tokens[34]);
  EXPECT_TOKEN_TYPE(Token::Type::SUPER, tokens[35]);
  EXPECT_TOKEN_TYPE(Token::Type::THIS, tokens[36]);
  EXPECT_TOKEN_TYPE(Token::Type::VAR, tokens[37]);
  EXPECT_TOKEN_TYPE(Token::Type::WHILE, tokens[38]);

  EXPECT_TOKEN_TYPE(Token::Type::DOT, tokens[39]);

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
  ASSERT_EQ(4, tokens.size());
  EXPECT_START(tokens);
  EXPECT_EQ("s1", tokens[1].getContents());
  EXPECT_EQ("s2", tokens[2].getContents());
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
  ASSERT_EQ(2, tokens.size());
  EXPECT_START(tokens);
  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestLexCommentAndEndOfLine) {
  Lexer lexer;
  const auto input = "// I am a comment\n";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(2, tokens.size());
  EXPECT_START(tokens);
  EXPECT_EOF(tokens);
}

// FIXME: update tests from here downwards, then update parser tests, then hopefully please be fixed.

TEST(LexerTests, TestLexCommentWithManySlashes) {
  Lexer lexer;
  const auto input = "/// I // am / a comment //";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(2, tokens.size());
  EXPECT_START(tokens);
  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestSingleAndMultiCharTokens) {
  Lexer lexer;
  const auto input = "!!===!<==>=/ /";
  const auto tokens = lexer.lex(input);
  ASSERT_EQ(11, tokens.size());

  EXPECT_START(tokens);

  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[1]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG_EQ, tokens[2]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ_EQ, tokens[3]);
  EXPECT_TOKEN_TYPE(Token::Type::BANG, tokens[4]);
  EXPECT_TOKEN_TYPE(Token::Type::LT_EQ, tokens[5]);
  EXPECT_TOKEN_TYPE(Token::Type::EQ, tokens[6]);
  EXPECT_TOKEN_TYPE(Token::Type::GT_EQ, tokens[7]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[8]);
  EXPECT_TOKEN_TYPE(Token::Type::SLASH, tokens[9]);

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestIdentifiers) {
  Lexer lexer;
  const auto input = "abc abc123 123abc _1212 _a_b_c a1a";
  const auto tokens = lexer.lex(input);

  EXPECT_START(tokens);

  expectIdentifierToken(tokens[1], "abc");
  expectIdentifierToken(tokens[2], "abc123");
  EXPECT_TOKEN_TYPE(Token::Type::NUM, tokens[3]);
  expectIdentifierToken(tokens[4], "abc");
  expectIdentifierToken(tokens[5], "_1212");
  expectIdentifierToken(tokens[6], "_a_b_c");
  expectIdentifierToken(tokens[7], "a1a");

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestIdentAndReservedWord) {
  Lexer lexer;
  const auto input = "formula andrew footprints foot print";
  const auto tokens = lexer.lex(input);

  EXPECT_START(tokens);

  expectIdentifierToken(tokens[1], "formula");
  expectIdentifierToken(tokens[2], "andrew");
  expectIdentifierToken(tokens[3], "footprints");
  expectIdentifierToken(tokens[4], "foot");
  EXPECT_TOKEN_TYPE(Token::Type::PRINT, tokens[5]);

  EXPECT_EOF(tokens);
}

TEST(LexerTests, TestLineNumbers) {
  Lexer lexer;
  const auto input = "abc\ndef ghi\n\njkl";
  const auto tokens = lexer.lex(input);

  EXPECT_START(tokens);

  expectIdentifierToken(tokens[1], "abc", 1);
  expectIdentifierToken(tokens[2], "def", 2);
  expectIdentifierToken(tokens[4], "jkl", 4);

  EXPECT_EOF(tokens);
}

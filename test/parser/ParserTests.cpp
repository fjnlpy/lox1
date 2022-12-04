#include "gtest/gtest.h"

#include <variant>
#include <vector>
#include <sstream>

#include "parser/Parser.h"
#include "visit/PrettyPrinter.hpp"

using lexer::Token;
using parser::Parser;
using namespace ast;

namespace {

void
assertProbablyTheSame(Expr &e1, Expr &e2)
{
  // We could write a pass which traverses two expressions and checks
  // that they're structurally the same. But I don't think we quite need
  // that right now. An easier approach is just to print them out
  // and say that they're the same iff they look the same.
  // (This also gives us nice assertion messages for free:).)
  // Possible sources of incorrectness: rounding of numbers, ...
  visit::PrettyPrinter printer;
  auto output1 = printer.print(e1);
  auto output2 = printer.print(e2);
  ASSERT_EQ(output1, output2);
}

void
assertDoesNotCompile(const std::vector<Token> &tokens)
{
  Parser parser;
  try {
    parser.parse(tokens);

    std::stringstream stream;
    stream << "Expected the sequence of tokens to fail parsing: ";
    if (tokens.size() == 0) {
      stream << "{}";
    } else {
      stream << "{\n  " << tokens[0];
      for (auto it = ++tokens.begin(); it != tokens.end(); ++it) {
        stream << ",\n  " << *it;
      }
      stream << "\n}";
    }
    FAIL() << stream.str();
  } catch (const CompileError &error) {
    // Test passed!
    //LOGD(e.what());
  }
}

}

TEST(ParserTests, TestPrimaryExpression) {
  Parser parser;

  auto expr = parser.parse(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::STR, "hello", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );

  ASSERT_EQ("hello", std::get<ast::StringPtr>(expr)->value());
}

TEST(ParserTests, TestOperatorPrecedence) {
  Parser parser;

  Expr actual = parser.parse({
    Token(Token::Type::START, "", std::nullopt),
    Token(Token::Type::NUM, "1", std::nullopt),
    Token(Token::Type::PLUS, "", std::nullopt),
    Token(Token::Type::NUM, "2", std::nullopt),
    Token(Token::Type::STAR, "", std::nullopt),
    Token(Token::Type::NUM, "3", std::nullopt),
    Token(Token::Type::PLUS, "", std::nullopt),
    Token(Token::Type::NUM, "4", std::nullopt),
    Token(Token::Type::EOFF, "", std::nullopt),
  });

  Expr expected = add(add(num(1), mult(num(2), num(3))), num(4));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestTrailingBinOp) {
  assertDoesNotCompile({
    Token(Token::Type::START, "", std::nullopt),
    Token(Token::Type::NUM, "1", std::nullopt),
    Token(Token::Type::EQ_EQ, "", std::nullopt),
    Token(Token::Type::EOFF, "", std::nullopt),
  });
}

TEST(ParserTests, TestGrouping) {
  Parser parser;

  Expr actual = parser.parse(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::LPEREN, "", std::nullopt),
      Token(Token::Type::NUM, "1", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::NUM, "2", std::nullopt),
      Token(Token::Type::RPEREN, "", std::nullopt),
      Token(Token::Type::STAR, "", std::nullopt),
      Token(Token::Type::NUM, "3", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );

  Expr expected = mult(grouping(sub(num(1), num(2))), num(3));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestUnclosedGroup) {
  assertDoesNotCompile(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::NUM, "1", std::nullopt),
      Token(Token::Type::PLUS, "", std::nullopt),
      Token(Token::Type::LPEREN, "", std::nullopt),
      Token(Token::Type::NUM, "2", std::nullopt),
      Token(Token::Type::PLUS, "", std::nullopt),
      Token(Token::Type::NUM, "3", std::nullopt),
      // Missing bracket:
      //Token(Token::Type::RPEREN, "", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );
}

TEST(ParserTests, TestNestedUnaryOps) {
  Parser parser;

  Expr actual = parser.parse(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::NUM, "1", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::NUM, "2", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );

  Expr expected = sub(negate(num(1)), negate(negate(num(2))));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestLeadingBinOp) {
  assertDoesNotCompile(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::SLASH, "", std::nullopt),
      Token(Token::Type::NUM, "10", std::nullopt),
      Token(Token::Type::MINUS, "", std::nullopt),
      Token(Token::Type::NUM, "9", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );
}

TEST(ParserTests, TestInvalidPrimaryExpression) {
  assertDoesNotCompile(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::NIL, "", std::nullopt),
      Token(Token::Type::NIL, "", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );
}

// I tried to use a long number here, hoping that the standard library would
// complain but it seems to just round the number to the closest thing
// it can represent. I'm not sure if that's always the case or if it's
// platform-specific, so I think it's good to have code in the parser for
// catching and forwarding the exceptions that the STL throws.
// And we need to test it somehow so just throw some junk in -- this should
// never get past the lexer in the first place.
TEST(ParserTests, TestUnsupportedNumber) {
  assertDoesNotCompile(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::NUM, "thisisnotanumber", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt)
    }
  );
}

TEST(ParserTests, TestEOFNotEndOfProgram) {
  assertDoesNotCompile(
    {
      Token(Token::Type::START, "", std::nullopt),
      Token(Token::Type::NUM, "1", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt), // This EOF should cause problems.
      Token(Token::Type::PLUS, "", std::nullopt),
      Token(Token::Type::NUM, "1", std::nullopt),
      Token(Token::Type::EOFF, "", std::nullopt),
    }
  );
}

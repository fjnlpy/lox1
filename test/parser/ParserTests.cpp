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
      Token(Token::Type::STR, 1, "hello"),
      Token(Token::Type::EOFF, 1, ""),
    }
  );

  ASSERT_EQ("hello", std::get<ast::StringPtr>(expr)->value());
}

TEST(ParserTests, TestOperatorPrecedence) {
  Parser parser;

  Expr actual = parser.parse({
    Token(Token::Type::NUM, 1, "1"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "2"),
    Token(Token::Type::STAR, 1, ""),
    Token(Token::Type::NUM, 1, "3"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "4"),
    Token(Token::Type::EOFF, 1, ""),
  });

  Expr expected = add(add(num(1), mult(num(2), num(3))), num(4));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestTrailingBinOp) {
  assertDoesNotCompile({
    Token(Token::Type::NUM, 1, "1"),
    Token(Token::Type::EQ_EQ, 1, ""),
    Token(Token::Type::EOFF, 1, ""),
  });
}

TEST(ParserTests, TestGrouping) {
  Parser parser;

  Expr actual = parser.parse(
    {
      Token(Token::Type::LPEREN, 1, ""),
      Token(Token::Type::NUM, 1, "1"),
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::NUM, 1, "2"),
      Token(Token::Type::RPEREN, 1, ""),
      Token(Token::Type::STAR, 1, ""),
      Token(Token::Type::NUM, 3, "3"),
      Token(Token::Type::EOFF, 1, ""),
    }
  );

  Expr expected = mult(grouping(sub(num(1), num(2))), num(3));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestUnclosedGroup) {
  assertDoesNotCompile(
    {
      Token(Token::Type::NUM, 1, "1"),
      Token(Token::Type::PLUS, 1, ""),
      Token(Token::Type::LPEREN, 1, ""),
      Token(Token::Type::NUM, 1, "2"),
      Token(Token::Type::PLUS, 1, ""),
      Token(Token::Type::NUM, 1, "3"),
      // Missing bracket:
      //Token(Token::Type::RPEREN, 1, ""),
      Token(Token::Type::EOFF, 1, ""),
    }
  );
}

TEST(ParserTests, TestNestedUnaryOps) {
  Parser parser;

  Expr actual = parser.parse(
    {
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::NUM, 1, "1"),
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::NUM, 1, "2"),
      Token(Token::Type::EOFF, 1, ""),
    }
  );

  Expr expected = sub(negate(num(1)), negate(negate(num(2))));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestLeadingBinOp) {
  assertDoesNotCompile(
    {
      Token(Token::Type::SLASH, 1, ""),
      Token(Token::Type::NUM, 1, "10"),
      Token(Token::Type::MINUS, 1, ""),
      Token(Token::Type::NUM, 1, "9"),
      Token(Token::Type::EOFF, 1, ""),
    }
  );
}

TEST(ParserTests, TestInvalidPrimaryExpression) {
  assertDoesNotCompile(
    {
      Token(Token::Type::NIL, 1, ""),
      Token(Token::Type::NIL, 1, ""),
      Token(Token::Type::EOFF, 1, ""),
    }
  );
}

TEST(ParserTests, TestUnsupportedNumber) {
  assertDoesNotCompile(
    {
      // I tried to use a long number here, hoping that the standard library would
      // complain but it seems to just round the number to the closest thing
      // it can represent. I'm not sure if that's always the case or if it's
      // platform-specific, so I think it's good to have code in the parser for
      // catching and forwarding the exceptions that the STL throws.
      // And we need to test it somehow so just throw some junk in -- this should
      // never get past the lexer in the first place.
      Token(Token::Type::NUM, 1, "thisisnotanumber"),
      Token(Token::Type::EOFF, 1, "")
    }
  );
}

TEST(ParserTests, TestEOFNotEndOfProgram) {
  assertDoesNotCompile(
    {
      Token(Token::Type::NUM, 1, "1"),
      Token(Token::Type::EOFF, 1, ""), // This EOF should cause problems.
      Token(Token::Type::PLUS, 1, ""),
      Token(Token::Type::NUM, 1, "1"),
      Token(Token::Type::EOFF, 1, ""),
    }
  );
}

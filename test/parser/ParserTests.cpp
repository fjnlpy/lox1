#include "gtest/gtest.h"

#include <variant>
#include <vector>
#include <sstream>

#include "parser/Parser.h"
#include "visit/PrettyPrinter.hpp"

using lexer::Token;
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
  parser::Parser parser;
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
  parser::Parser parser;

  auto expr = parser.parse({Token(Token::Type::STR, 1, "hello")});

  ASSERT_EQ("hello", std::get<ast::StringPtr>(expr)->value());
}

TEST(ParserTests, TestOperatorPrecedence) {
  parser::Parser parser;

  Expr actual = parser.parse({
    Token(Token::Type::NUM, 1, "1"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "2"),
    Token(Token::Type::STAR, 1, ""),
    Token(Token::Type::NUM, 1, "3"),
    Token(Token::Type::PLUS, 1, ""),
    Token(Token::Type::NUM, 1, "4")
  });

  Expr expected = add(add(num(1), mult(num(2), num(3))), num(4));

  assertProbablyTheSame(actual, expected);
}

TEST(ParserTests, TestTrailingBinOp) {
  assertDoesNotCompile({
    Token(Token::Type::NUM, 1, "1"),
    Token(Token::Type::EQ_EQ, 1, ""),
  });
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

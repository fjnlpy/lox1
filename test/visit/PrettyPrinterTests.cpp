#include <gtest/gtest.h>

#include <string>

#include "visit/PrettyPrinter.hpp"
#include "utils/Logging.hpp"

TEST(PrettyPrinterTests, TestDigits) {
  visit::PrettyPrinter printer;

  ast::Expr num = ast::num(2.505);
  auto printedNum = printer.print(num);

  ASSERT_EQ("2.505", printedNum);
}

TEST(PrettyPrinterTests, TestRounding) {
  visit::PrettyPrinter printer;

  ast::Expr num = ast::num(2.5059);
  auto printedNum = printer.print(num);

  ASSERT_EQ("2.506", printedNum);
}

TEST(PrettyPrinterTests, TestDropTrailingZerosOnly) {
  visit::PrettyPrinter printer;

  ast::Expr num = ast::num(2.0499);
  auto printedNum = printer.print(num);

  ASSERT_EQ("2.05", printedNum);
}

TEST(PrettyPrinterTests, TestDropTrailingZerosAndDot) {
  visit::PrettyPrinter printer;

  ast::Expr num = ast::num(2.9999);
  auto printedNum = printer.print(num);

  ASSERT_EQ("3", printedNum);
}

TEST(PrettyPrinterTests, TestString) {
  visit::PrettyPrinter printer;

  ast::Expr string = ast::string("a b c 123");
  auto printedString = printer.print(string);

  ASSERT_EQ("\"a b c 123\"", printedString);
}

TEST(PrettyPrinterTests, TestBigTree) {
  visit::PrettyPrinter printer;

  using namespace ast;

  Expr expr = mult(add(num(1.5), num(2)), grouping(negate(string("1 1 1"))));
  auto printedExpr = printer.print(expr);

  ASSERT_EQ("(* (+ 1.5 2) (group (- \"1 1 1\")))", printedExpr);
}

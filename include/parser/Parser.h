#pragma once

#include <vector>
#include <optional>
#include <functional>

#include "ast/Expr.hpp"
#include "lexer/Lexer.h"

namespace parser {

class Parser {
public:

  ast::Expr parse(std::vector<lexer::Token> tokens);

private:

  size_t current_;
  std::vector<lexer::Token> tokens_;

  // Helpers for scanning through tokens.
  const lexer::Token &current() const;
  const lexer::Token &advance();
  template <class... T> bool peek(T &&...);
  template <class... T> std::optional<std::reference_wrapper<const lexer::Token>> match(T &&...);
  template <class... T> const lexer::Token &expect(T &&...);

  // Helpers for parsing grammar structures into specific AST nodes.
  template <class BinOpMapFunc, class SubExprFunc, class... Ts>
  ast::Expr createBinOp(const BinOpMapFunc &, const SubExprFunc &, Ts &&...);

  // Helpers for error reporting.
  double textToDouble(const std::string &);

  // Methods for non-terminals in the grammar.
  ast::Expr expression();
  ast::Expr equality();
  ast::Expr comparison();
  ast::Expr term();
  ast::Expr factor();
  ast::Expr unary();
  ast::Expr primary();

};

}

#pragma once

#include <vector>

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
  template <class... T> bool peek(T &&... tokens);
  template <class... T> bool match(T &&... tokens);


  // Methods for non-terminals in the grammar.
  ast::Expr expression();
  ast::Expr equality();
  ast::Expr comparison();

};

}

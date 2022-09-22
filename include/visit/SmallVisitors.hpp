#pragma once

#include "ast/Expr.hpp"
#include <variant>

namespace visit {

auto
id(const ast::Expr &expr)
{
  return std::visit([](auto &node) { return node->id(); }, expr);
}

}

#pragma once

#include <variant>
#include <string>
#include <memory>
#include <utility>

#include "utils/Counter.hpp"

namespace ast {

// Forward declarations are needed because the superclass and subclasses mutually refer to each other.
struct BinOp;
struct UnaryOp;
struct String;
struct Num;
struct Grouping;

using Expr = std::variant<
  std::unique_ptr<BinOp>,
  std::unique_ptr<UnaryOp>,
  std::unique_ptr<String>,
  std::unique_ptr<Num>,
  std::unique_ptr<Grouping>
>;


class BinOp {
public:
  enum class Op { Mult, Div, Add, Sub, GtEq, Gt, LtEq, Lt, Eq, Neq };

  BinOp(
    Expr lhs,
    Op operation,
    Expr rhs,
    size_t id
  ): lhs_(std::move(lhs)),
    operation_(std::move(operation)),
    rhs_(std::move(rhs)),
    id_(std::move(id)) { }

  Expr &lhs() { return lhs_; }
  Op &operation() { return operation_; }
  Expr &rhs() { return rhs_; }
  size_t &id() { return id_; }

private:
  Expr lhs_;
  Op operation_;
  Expr rhs_;
  size_t id_;
};


class UnaryOp {
public:
  enum class Op { Negate, Nott };

  UnaryOp(
    Op operation,
    Expr child,
    size_t id
  ): operation_(std::move(operation)),
    child_(std::move(child)),
    id_(std::move(id)) { }

  Op &operation() { return operation_; }
  Expr &child() { return child_; }
  size_t &id() { return id_; }

private:
  Op operation_;
  Expr child_;
  size_t id_;
};


class String {
public:

  String(
    std::string value,
    size_t id
  ): value_(std::move(value)),
    id_(std::move(id)) { }

  std::string &value() { return value_; }
  size_t &id() { return id_; }

private:
  std::string value_;
  size_t id_;
};


class Num {
public:

  Num(
    double value,
    size_t id
  ): value_(std::move(value)),
    id_(std::move(id)) { }

  double &value() { return value_; }
  size_t &id() { return id_; }

private:
  double value_;
  size_t id_;
};


class Grouping {
public:

  Grouping(
    Expr child,
    size_t id
  ): child_(std::move(child)),
    id_(std::move(id)) { }

  Expr &child() { return child_; }
  size_t &id() { return id_; }

private:
  Expr child_;
  size_t id_;
};

std::unique_ptr<BinOp>
inline mult(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Mult,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline div(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Div,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline add(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Add,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline sub(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Sub,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline gtEq(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::GtEq,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline gt(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Gt,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline ltEq(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::LtEq,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline lt(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Lt,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline eq(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Eq,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<BinOp>
inline neq(
  Expr &&lhs,
  Expr &&rhs
) {
  return std::make_unique<
    BinOp,
    Expr,
    BinOp::Op,
    Expr,
    size_t
  >(
    std::move(lhs),
    BinOp::Op::Neq,
    std::move(rhs),
    Counter::next()
  );
}


std::unique_ptr<UnaryOp>
inline negate(
  Expr &&child
) {
  return std::make_unique<
    UnaryOp,
    UnaryOp::Op,
    Expr,
    size_t
  >(
    UnaryOp::Op::Negate,
    std::move(child),
    Counter::next()
  );
}


std::unique_ptr<UnaryOp>
inline nott(
  Expr &&child
) {
  return std::make_unique<
    UnaryOp,
    UnaryOp::Op,
    Expr,
    size_t
  >(
    UnaryOp::Op::Nott,
    std::move(child),
    Counter::next()
  );
}


std::unique_ptr<String>
inline string(
  std::string &&value
) {
  return std::make_unique<
    String,
    std::string,
    size_t
  >(
    std::move(value),
    Counter::next()
  );
}


std::unique_ptr<Num>
inline num(
  double &&value
) {
  return std::make_unique<
    Num,
    double,
    size_t
  >(
    std::move(value),
    Counter::next()
  );
}


std::unique_ptr<Grouping>
inline grouping(
  Expr &&child
) {
  return std::make_unique<
    Grouping,
    Expr,
    size_t
  >(
    std::move(child),
    Counter::next()
  );
}

template <class T>
class Visitor {
public:
  virtual ~Visitor() =default;

  virtual T visitBinOp(BinOp &binOp) =0;
  T operator()(std::unique_ptr<BinOp> &binOp) { return visitBinOp(*binOp); }

  virtual T visitUnaryOp(UnaryOp &unaryOp) =0;
  T operator()(std::unique_ptr<UnaryOp> &unaryOp) { return visitUnaryOp(*unaryOp); }

  virtual T visitString(String &string) =0;
  T operator()(std::unique_ptr<String> &string) { return visitString(*string); }

  virtual T visitNum(Num &num) =0;
  T operator()(std::unique_ptr<Num> &num) { return visitNum(*num); }

  virtual T visitGrouping(Grouping &grouping) =0;
  T operator()(std::unique_ptr<Grouping> &grouping) { return visitGrouping(*grouping); }

  T
  visit(Expr &expr)
  {
    return std::visit(*this, expr);
  }

};
}

#ifndef BOOLEAN_EXPR_HPP
#define BOOLEAN_EXPR_HPP

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <cassert>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace boolean {

struct Expr0 {
  enum Op {
    Const = 0,
    Var = 1,
    Not = 2,
    And = 3,
    Or = 4
  };

  typedef uint32_t Kind;
  typedef uint32_t Ref;

  union {
    struct {
      Kind kind;
      Ref ref;
    };
    uint64_t value;
  };

  bool operator== (Expr0 u) const {
    return u.value == value;
  }
  bool operator!= (Expr0 u) const {
    return u.value != value;
  }
  bool operator< (Expr0 u) const {
    return u.value < value;
  }
};

struct Expr0Hash {
  size_t operator()(const Expr0& s) const noexcept
  {
    return s.value;
  }
};

struct Expr0Expr0Hash {
  size_t operator()(const std::tuple<Expr0, Expr0>& s) const noexcept
  {
    size_t h1 = Expr0Hash{}(std::get<0>(s));
    size_t h2 = Expr0Hash{}(std::get<1>(s));
    return h1 ^ (h2 << 1);
  }
};

class Context {
  std::unordered_map<std::tuple<Expr0,Expr0>, Expr0, Expr0Expr0Hash> to_and;
  std::unordered_map<std::tuple<Expr0,Expr0>, Expr0, Expr0Expr0Hash> to_or;
  std::unordered_map<Expr0, Expr0, Expr0Hash> to_not;
  std::vector<std::tuple<Expr0, Expr0>> exprs;

  static constexpr Expr0 trueExpr0{Expr0::Const, 1};
  static constexpr Expr0 falseExpr0{Expr0::Const, 0};

public:
  Context() {
    exprs.reserve(1024*64);
  }

  std::tuple<Expr0, Expr0> get_args(Expr0 expr) const {
    assert(expr.ref < exprs.size());
    return exprs[expr.ref];
  }

  Expr0 new_value(bool v) {
    return v ? trueExpr0 : falseExpr0;
  }

  Expr0 new_var(uint32_t var) {
    return Expr0 { Expr0::Var, var };
  }

  Expr0 new_not(Expr0 e) {
    Expr0 e1{Expr0::Not, (uint32_t)exprs.size()};
    auto [i,has] = to_not.insert({e, e1});
    if (has) {
      exprs.push_back({e,e});
    }
    return i->second;
  }

  Expr0 new_and(Expr0 l, Expr0 r) {
    Expr0 e1{Expr0::And, (uint32_t)exprs.size()};
    auto [i, has] = to_and.insert({{l,r}, e1});
    if (has) {
      exprs.push_back({l,r});
    }
    return i->second;
  }

  Expr0 new_or(Expr0 l, Expr0 r) {
    Expr0 e1{Expr0::Or, (uint32_t)exprs.size()};
    auto [i, has] = to_or.insert({{l,r}, e1});
    if (has) {
      exprs.push_back({l,r});
    }
    return i->second;
  }
};

class Expr {
public:
  Expr() {}
  Expr(const Expr& expr1) : expr(expr1.expr) {}
  Expr(Expr0 expr0) : expr(expr0) {}

  Expr& operator=(const Expr& expr1) { expr = expr1.expr; return *this; }

  bool is_const() const { return expr.kind == Expr0::Const; }
  bool is_var() const { return expr.kind == Expr0::Var; }
  bool is_not() const { return expr.kind == Expr0::Not; }
  bool is_and() const { return expr.kind == Expr0::And; }
  bool is_or() const { return expr.kind == Expr0::Or; }

  bool get_value(bool& v) const {
    if (expr.kind == Expr0::Const) {
      v = expr.ref != 0;
      return true;
    }
    return false;
  }

  bool get_var(uint32_t& var) const {
    if (expr.kind == Expr0::Var) {
      var = expr.ref;
      return true;
    }
    return false;
  }

  bool not_arg(Expr& arg) const {
    if (expr.kind != Expr0::Not) {
      return false;
    }
    arg = Expr{std::get<0>(context.get_args(expr))};
    return true;
  }

  bool and_args(Expr& lhs, Expr& rhs) const {
    if (expr.kind != Expr0::And) {
      return false;
    }
    auto [lhs0, rhs0] = context.get_args(expr);
    lhs = Expr{lhs0};
    rhs = Expr{rhs0};
    return true;
  }

  bool or_args(Expr& lhs, Expr& rhs) const {
    if (expr.kind != Expr0::Or) {
      return false;
    }
    auto [lhs0, rhs0] = context.get_args(expr);
    lhs = Expr{lhs0};
    rhs = Expr{rhs0};
    return true;
  }

  uint32_t var_count() const {
    uint32_t v;
    Expr lhs, rhs;
    if (get_var(v)) {
      return v + 1;
    }
    if (not_arg(lhs)) {
      return lhs.var_count();
    }
    if (and_args(lhs, rhs)) {
      return std::max(lhs.var_count(), rhs.var_count());
    }
    if (or_args(lhs, rhs)) {
      return std::max(lhs.var_count(), rhs.var_count());
    }
    return 0;
  }

  std::string pretty() const { assert(false); return ""; } // not implemented

  static Expr value (bool v) { return Expr{context.new_value(v)}; }
  static Expr var (uint32_t v) { return Expr{context.new_var(v)}; }
  friend bool operator == (Expr lhs, Expr rhs) {
    return lhs.expr == rhs.expr;
  }
  friend bool operator != (Expr lhs, Expr rhs) {
    return lhs.expr != rhs.expr;
  }
  friend Expr operator! (Expr expr) {
    return Expr::context.new_not(expr.expr);
  }
  friend Expr operator&& (Expr lhs, Expr rhs) {
    return Expr::context.new_and(lhs.expr, rhs.expr);
  }
  friend Expr operator|| (Expr lhs, Expr rhs) {
    return Expr::context.new_or(lhs.expr, rhs.expr);
  }
  friend Expr implies(Expr lhs, Expr rhs) {
    return !lhs || rhs;
  }

private:
  Expr0 expr;
  static Context context;
};

extern void from_json(const json& j, Expr& e);
extern void to_json(json& j, const Expr& a);
extern void toRtPredicate(Expr expr,
                          std::vector<uint8_t>& data);
} // boolean

#endif // BOOLEAN_EXPR_HPP

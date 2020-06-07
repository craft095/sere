#ifndef BOOLEAN_EXPR_HPP
#define BOOLEAN_EXPR_HPP

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <cassert>

#include <tsl/robin_map.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace boolean {

  /*
   * Some observations
   * - expressions are created much more times than traversed
   * - double word represenation does matter
   * - caches (even for large SERE) are of moderate size
   * - this implementation is _single_ threaded
   */

struct Expr0 {
  enum Op {
    NoValue = 0, // ref is unused
    Const = 1, // ref is (~0 - (0 | 1))
    Var = 2, // ref is (~0 - (varId + 2))
    Not = 3, // ref is used as an index
    And = 4, // ref is used as an index
    Or = 5   // ref is used as an index
  };

  typedef uint32_t Kind;
  typedef uint32_t Ref;

  static constexpr uint32_t const_base = ~0;
  static constexpr uint32_t var_base = const_base - 2;

  union {
    struct {
      Kind kind;
      Ref ref; // ref is unique id of an expression
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

enum Func
  {
   F_NNF = 0,
   F_SAT = 1,
   F_SIMPLIFY = 2,
   F_FUNC_COUNT = 3
  };

  /**
   * Operation key uniquely identifies pair of expressions.
   * The idea here is to pack the key into UInt64.
   * For this purpose Ref (which UInt32) in Expr0 was made globally unique.
   */
  union OpKey {
    struct {
      Expr0::Ref lhs;
      Expr0::Ref rhs;
    };
    uint64_t key;
  };

  inline bool operator== (OpKey x, OpKey y) { return x.key == y.key; }

  inline uint32_t hashAmiga(uint32_t value) {
    return value * 0xdeece66d + 0xb;
  }

  struct UInt32Hash {
    size_t operator()(uint32_t k) const noexcept {
      return hashAmiga(k);
    }
  };

  struct OpKeyHash {
    size_t operator()(OpKey k) const noexcept {
      // Cantor pairing function
      return k.rhs + (k.lhs + k.rhs)*(k.lhs + k.rhs + 1)/2;
      //return k.key; //k.rhs + (k.lhs + k.rhs)*(k.lhs + k.rhs + 1)/2;
      //return k.key*0x517cc1b727220a95;
      /*
      size_t h1 = hashAmiga(k.lhs);
      size_t h2 = hashAmiga(k.rhs);
      return h1 ^ (h2 << 1);*/
    }
  };

class Context {
  typedef tsl::robin_map<OpKey, Expr0, OpKeyHash> E2Map;
  typedef tsl::robin_map<Expr0::Ref, Expr0> EMap;
  // std::unordered_map<OpKey, Expr0, OpKeyHash> to_and;
  // std::unordered_map<OpKey, Expr0, OpKeyHash> to_or;
  // std::unordered_map<Expr0::Ref, Expr0, UInt32Hash> to_not;
  E2Map to_and;
  E2Map to_or;
  EMap to_not;
  std::vector<std::tuple<Expr0, Expr0>> exprs;

  typedef std::vector<Expr0> RefFunc;
  typedef std::array<RefFunc, F_FUNC_COUNT> RefFuncs;

  RefFuncs funcs;

  static constexpr Expr0 trueExpr0{Expr0::Const, Expr0::const_base - 1};
  static constexpr Expr0 falseExpr0{Expr0::Const, Expr0::const_base};

  void add_expr(std::tuple<Expr0, Expr0> texpr) {
    exprs.push_back(texpr);
    for (auto& e : funcs) {
      assert(exprs.size() == e.size() + 1);
      e.push_back(novalue());
    }
  }

public:
  Context() {
    to_and.reserve(1024*1024*4);
    to_or.reserve(1024*1024*4);
    to_not.reserve(1024*64);
    exprs.reserve(1024*64);
    for (auto& e : funcs) {
      e.reserve(1024*64);
    }
    //add_expr({novalue(),{0}}); // novalue
  }

  template <typename F>
  Expr0 query_and_update_func(Func func, Expr0 arg, F f) {
    Expr0 r = funcs[func][arg.ref];
    if (r == novalue()) {
      r = f(arg);
      funcs[func][arg.ref] = r;
    }
    return r;
  }

  Expr0& func_site(Expr0 expr, Func func) {
    return funcs[func][expr.ref];
  }

  std::tuple<Expr0, Expr0> get_args(Expr0 expr) const {
    assert(expr.ref < exprs.size());
    return exprs[expr.ref];
  }

  static Expr0 novalue() {
    return Expr0{{0,0}};
  }

  Expr0 new_value(bool v) {
    return v ? trueExpr0 : falseExpr0;
  }

  Expr0 new_var(uint32_t var) {
    return Expr0 { Expr0::Var, Expr0::var_base - var };
  }

  Expr0 new_not(Expr0 e) {
    Expr0 e1{Expr0::Not, (uint32_t)exprs.size()};
    auto [i,is_new] = to_not.insert({e.ref, e1});
    if (is_new) {
      add_expr({e,novalue()});
    }
    return i->second;
  }

  Expr0 new_and(Expr0 l, Expr0 r) {
    Expr0 e1{Expr0::And, (uint32_t)exprs.size()};

    if (l.ref > r.ref) {
      std::swap(l.value, r.value);
    }

    OpKey k {l.ref, r.ref};
    auto [i, is_new] = to_and.insert({k, e1});
    if (is_new) {
      add_expr({l,r});
    }
    return i->second;
  }

  Expr0 new_or(Expr0 l, Expr0 r) {
    Expr0 e1{Expr0::Or, (uint32_t)exprs.size()};

    if (l.ref > r.ref) {
      std::swap(l.value, r.value);
    }

    OpKey k {l.ref, r.ref};
    auto [i, is_new] = to_or.insert({k, e1});
    if (is_new) {
      add_expr({l,r});
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

  /**
   * Caching predefined set of Expr->Expr functions
   *
   * Can only be applied to non-terms: not?, ?or?, ?and?
   */

  template <typename F>
  Expr query_and_update_func(Func func, F f) const {
    assert(!is_const() && !is_var());
    return context.query_and_update_func
      (
       func,
       expr,
       [&f](Expr0 a) {
         return f(Expr{a}).expr;
       }
       );
  }

  bool is_const() const { return expr.kind == Expr0::Const; }
  bool is_var() const { return expr.kind == Expr0::Var; }
  bool is_not() const { return expr.kind == Expr0::Not; }
  bool is_and() const { return expr.kind == Expr0::And; }
  bool is_or() const { return expr.kind == Expr0::Or; }

  bool get_value(bool& v) const {
    if (expr.kind == Expr0::Const) {
      v = expr.ref != Expr0::const_base;
      return true;
    }
    return false;
  }

  bool get_var(uint32_t& var) const {
    if (expr.kind == Expr0::Var) {
      var = Expr0::var_base - expr.ref;
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

  //private:
  Expr0 expr;
  static Context context;
};

extern void from_json(const json& j, Expr& e);
extern void to_json(json& j, const Expr& a);
extern void toRtPredicate(Expr expr,
                          std::vector<uint8_t>& data);
} // boolean

#endif // BOOLEAN_EXPR_HPP

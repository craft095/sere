#include "c++/z3++.h"

#include "Letter.hpp"
#include "Language.hpp"

#include "Z3.hpp"
#include "TestZ3.hpp"

z3::expr letterToZex(const Letter& l) {
  z3::expr e(theContext.bool_val(true));
  for (const auto& p : l.pos) {
    e = e && theContext.bool_const(p.pretty());
  }
  for (const auto& n : l.neg) {
    e = e && !theContext.bool_const(n.pretty());
  }
  return e;
}

bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_) {
  z3::expr r = implies(vars_, expr_);
  return prove(r);
}

bool evalWithImply0(const Letter& l, const z3::expr& expr_) {
  z3::solver s(theContext);
  s.add(expr_);
  for (auto v : l.pos) {
    s.add(theContext.bool_const(v.pretty()));
  }
  for (auto v : l.neg) {
    s.add(!theContext.bool_const(v.pretty()));
  }
  return s.check() == z3::sat;
}

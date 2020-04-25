#include "test/Letter.hpp"
#include "test/ToolsZ3.hpp"

#include "ast/Common.hpp"
#include "sat/Z3.hpp"

#include "z3++.h"

z3::expr letterToZex(const rt::Names& names) {
  z3::expr e(theContext.bool_val(true));

  for (size_t k = 0; k < names.size(); ++k) {
    z3::expr e1 = theContext.bool_const(prettyName(k).c_str());
    if (names.test(k)) {
      e = e && e1;
    } else {
      e = e && !e1;
    }
  }
  return e;
}

bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_) {
  z3::expr r = implies(vars_, expr_);
  return prove(r);
}

#if 0
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
#endif

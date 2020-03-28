#include "c++/z3++.h"

#include "Letter.hpp"
#include "Language.hpp"

#include "Z3.hpp"

z3::context theContext;

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

class Z3Bool : public BoolVisitor {
private:
  z3::expr expr;
public:
  Z3Bool(BoolExpr& expr_) : expr(theContext) {
    expr_.accept(*this);
  }

  z3::expr getExpr() const { return expr; }

  void visit(Variable& v) override {
    expr = theContext.bool_const(v.getName().pretty());
  }

  void visit(BoolValue& v) override {
    expr = theContext.bool_val(v.getValue());
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    expr = !expr;
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    z3::expr lhs = expr;

    v.getRhs()->accept(*this);
    z3::expr rhs = expr;

    expr = lhs && rhs;
  }
};

z3::expr boolSereToZex(BoolExpr& be) {
  return Z3Bool(be).getExpr();
}

bool prove(const z3::expr& e) {
  z3::solver s(theContext);
  s.add(!e);
  return s.check() == z3::unsat;
}

bool satisfiable(const z3::expr& e) {
  z3::solver s(theContext);
  s.add(e);
  return s.check() != z3::unsat;
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

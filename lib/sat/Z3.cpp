#include "z3++.h"

#include "ast/BoolExpr.hpp"

#include "sat/Z3.hpp"

z3::context theContext;

class Z3Bool : public BoolVisitor {
private:
  z3::expr expr;
public:
  Z3Bool(BoolExpr& expr_) : expr(theContext) {
    expr_.accept(*this);
  }

  z3::expr getExpr() const { return expr; }

  void visit(Variable& v) override {
    expr = theContext.bool_const(v.getName().pretty().c_str());
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

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    z3::expr lhs = expr;

    v.getRhs()->accept(*this);
    z3::expr rhs = expr;

    expr = lhs || rhs;
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

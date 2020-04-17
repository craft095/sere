#include "Language.hpp"

#include <map>
#include <minisat/core/Solver.h>

using Minisat::mkLit;

/**
 * Tseitin translation
 * 1) Remove all true/false constants
 * 2) Reduce all not (not expr)
 * 3) Apply Tseitin traslation
 */

class Tseitin {
private:
  Minisat::Solver solver;

public:
  enum class Code : uint16_t {
    NAnd, // C <-> A & B
    And,  // C <-> !(A & B)
    Or,   // C <-> A | B
    NOr,  // C <-> !(A | B)
    Not,  // C <-> !A
  };

  Tseitin() {}

  bool solve() {
    return solver.solve();
  }

  void setRootVar(Minisat::Var var) {
    solver.addClause(mkLit(var));
  }

  Minisat::Var newVar() {
    return solver.newVar();
  }

  Minisat::Var addSlot(Code op, Minisat::Var A, Minisat::Var B = -1) {
    auto C = solver.newVar();

    switch (op) {
    case Code::Not:
      // C = ! A
      // (!A || !C) && (A || C)
      solver.addClause(~mkLit(A), ~mkLit(C));
      solver.addClause( mkLit(A),  mkLit(C));
      break;
    case Code::And:
      // C = A & B
      // (!A || !B || C) && (A || !C) && (B || !C)
      solver.addClause(~mkLit(A), ~mkLit(B),  mkLit(C));
      solver.addClause( mkLit(A), ~mkLit(C));
      solver.addClause( mkLit(B), ~mkLit(C));
      break;
    case Code::Or:
      // C = A & B
      // (A || B || !C) && (!A || C) && (!B || C)
      solver.addClause( mkLit(A),  mkLit(B), ~mkLit(C));
      solver.addClause(~mkLit(A),  mkLit(C));
      solver.addClause(~mkLit(B),  mkLit(C));
      break;
    default:
      assert(false); //unreachable code
    }
    return C;
  }
};

class TseitinBoolExpr : public BoolVisitor {
private:
  std::map<VarName, Minisat::Var> vars;
  Tseitin tseitin;
  Minisat::Var result; // variable assigned to a subexpression
  bool sat;

  using Code = Tseitin::Code;
public:
  TseitinBoolExpr(BoolExpr& expr) {
    if (auto u = dynamic_cast<BoolValue*>(&expr)) {
      sat = u->getValue();
    } else if (dynamic_cast<Variable*>(&expr)) {
      sat = true;
    } else {
      expr.accept(*this);
      tseitin.setRootVar(result);
      sat = tseitin.solve();
    }
  }

  bool isSatisfiable() const {
    return sat;
  }

  void visit(Variable& v) override {
    auto i = vars.find(v.getName());
    if (i == vars.end()) {
      result = tseitin.newVar();
      vars[v.getName()] = result;
    } else {
      result = i->second;
    }
  }

  void visit(BoolValue& ) override {
    assert(false); // unreachable code
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    auto arg = result;

    result = tseitin.addSlot(Code::Not, arg);
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    auto lhs = result;
    v.getRhs()->accept(*this);
    auto rhs = result;

    result = tseitin.addSlot(Code::And, lhs, rhs);
  }

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    auto lhs = result;
    v.getRhs()->accept(*this);
    auto rhs = result;

    result = tseitin.addSlot(Code::Or, lhs, rhs);
  }
};

class TseitinExpr {
private:
  std::map<uint32_t, Minisat::Var> vars;
  Tseitin tseitin;
  bool sat;

  using Code = Tseitin::Code;
public:
  TseitinExpr(expr::Expr expr) {
    bool v;
    if (expr.get_value(v)) {
      sat = v;
    } else if (expr.is_var()) {
      sat = true;
    } else {
      auto root = traverse(expr);
      tseitin.setRootVar(root);
      sat = tseitin.solve();
    }
  }

  bool isSatisfiable() const {
    return sat;
  }

  Minisat::Var traverse(expr::Expr expr) {
    uint32_t v;
    expr::Expr lhs, rhs;

    assert(!expr.is_const());

    if (expr.get_var(v)) {
      auto i = vars.find(v);
      if (i == vars.end()) {
        auto result = tseitin.newVar();
        vars[v] = result;
        return result;
      } else {
        return i->second;
      }
    }
    if (expr.not_arg(lhs)) {
      auto arg = traverse(lhs);
      return tseitin.addSlot(Code::Not, arg);
    }
    if (expr.and_args(lhs, rhs)) {
      auto lhsE = traverse(lhs);
      auto rhsE = traverse(rhs);
      return tseitin.addSlot(Code::And, lhsE, rhsE);
    }
    if (expr.or_args(lhs, rhs)) {
      auto lhsE = traverse(lhs);
      auto rhsE = traverse(rhs);
      return tseitin.addSlot(Code::Or, lhsE, rhsE);
    }
    assert(false); // unreachable code
  }
};

class Simplify : public BoolVisitor {
private:
  Ptr<BoolExpr> result;

public:
  Simplify(BoolExpr& expr) {
    expr.accept(*this);
  }

  Ptr<BoolExpr> getResult() const {
    return result;
  }

  void visit(Variable& v) override {
    result = RE_VAR(v.getName().ix);
  }

  void visit(BoolValue& v) override {
    result = v.getValue() ? RE_TRUE : RE_FALSE;
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    auto r = getResult();
    if (auto u = dynamic_cast<BoolNot*>(&*r)) {
      result = u->getArg();
    } else if (auto u = dynamic_cast<BoolValue*>(&*r)) {
      result = u->getValue() ? RE_FALSE : RE_TRUE;
    } else {
      result = RE_NOT(r);
    }
  }

  void visit(BoolAnd& v) override {
    Ptr<BoolExpr> lhs, rhs;

    v.getLhs()->accept(*this);
    lhs = result;
    v.getRhs()->accept(*this);
    rhs = result;

    if (auto lu = dynamic_cast<BoolValue*>(&*lhs)) {
      if (lu->getValue()) {
        result = rhs;
      } else {
        result = lhs;
      }
      return;
    }

    if (auto ru = dynamic_cast<BoolValue*>(&*rhs)) {
      if (ru->getValue()) {
        result = lhs;
      } else {
        result = rhs;
      }
      return;
    }

    result = RE_AND(lhs, rhs);
  }

  void visit(BoolOr& v) override {
    Ptr<BoolExpr> lhs, rhs;

    v.getLhs()->accept(*this);
    lhs = result;
    v.getRhs()->accept(*this);
    rhs = result;

    if (auto lu = dynamic_cast<BoolValue*>(&*lhs)) {
      if (lu->getValue()) {
        result = lhs;
      } else {
        result = rhs;
      }
      return;
    }

    if (auto ru = dynamic_cast<BoolValue*>(&*rhs)) {
      if (ru->getValue()) {
        result = rhs;
      } else {
        result = lhs;
      }
      return;
    }

    result = RE_OR(lhs, rhs);
  }
};

expr::Expr simplify(expr::Expr expr) {
  bool b;
  expr::Expr lhs, rhs;

  if (expr.is_const()) {
    return expr;
  }

  if (expr.is_var()) {
    return expr;
  }

  if (expr.not_arg(lhs)) {
    auto arg = simplify(lhs);

    if (arg.get_value(b)) {
      return expr::Expr::value(!b);
    }

    expr::Expr inner;
    if (arg.not_arg(inner)) {
      return inner;
    }

    return !arg;
  }

  if (expr.and_args(lhs, rhs)) {
    auto lhsE = simplify(lhs);
    auto rhsE = simplify(rhs);

    if (lhsE.get_value(b)) {
      return b ? rhsE : lhsE;
    }

    if (rhsE.get_value(b)) {
      return b ? lhsE : rhsE;
    }

    return lhsE && rhsE;
  }

  if (expr.or_args(lhs, rhs)) {
    auto lhsE = simplify(lhs);
    auto rhsE = simplify(rhs);

    if (lhsE.get_value(b)) {
      return b ? lhsE : rhsE;
    }

    if (rhsE.get_value(b)) {
      return b ? rhsE : lhsE;
    }

    return lhsE || rhsE;
  }

  assert(false); // unreachable code
}

bool sat(BoolExpr& expr) {
  auto simplifiedExpr = Simplify(expr).getResult();
  return TseitinBoolExpr(*simplifiedExpr).isSatisfiable();
}

bool sat(expr::Expr expr) {
  auto simplifiedExpr = simplify(expr);
  return TseitinExpr(simplifiedExpr).isSatisfiable();
}

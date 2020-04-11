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

class Tseitin : public BoolVisitor {
private:
  enum class Code : uint16_t {
    NAnd, // C <-> A & B
    And,  // C <-> !(A & B)
    Or,   // C <-> A | B
    NOr,  // C <-> !(A | B)
    Not,  // C <-> !A
  };

  std::map<VarName, Minisat::Var> vars;
  Minisat::Solver solver;
  Minisat::Var result; // variable assigned to a subexpression

  bool sat;

public:
  Tseitin(BoolExpr& expr) {
    if (auto u = dynamic_cast<BoolValue*>(&expr)) {
      sat = u->getValue();
      // slots.push_back(mkValue(u));
    } else if (dynamic_cast<Variable*>(&expr)) {
      sat = true;
    } else {
      expr.accept(*this);
      solver.addClause(mkLit(result));
      sat = solver.solve();
    }
  }

  bool isSatisfiable() const {
    return sat;
  }

  void visit(Variable& v) override {
    auto i = vars.find(v.getName());
    if (i == vars.end()) {
      result = solver.newVar();
      vars[v.getName()] = result;
    } else {
      result = i->second;
    }
  }

  void visit(BoolValue& ) override {
    assert(false); // unreachable code
  }

  void addSlot(Code op, Minisat::Var A, Minisat::Var B = -1) {
    result = solver.newVar();

    auto C = result;

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
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    auto arg = result;

    addSlot(Code::Not, arg);
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    auto lhs = result;
    v.getRhs()->accept(*this);
    auto rhs = result;

    addSlot(Code::And, lhs, rhs);
  }

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    auto lhs = result;
    v.getRhs()->accept(*this);
    auto rhs = result;

    addSlot(Code::Or, lhs, rhs);
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

bool sat(BoolExpr& expr) {
  auto simplifiedExpr = Simplify(expr).getResult();
  return Tseitin(*simplifiedExpr).isSatisfiable();
}

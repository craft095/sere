#include "sat/Cnf.hpp"
#include "ast/BoolExpr.hpp"
#include "boolean/Expr.hpp"

#include <map>
#include "minisat/core/Solver.h"

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
  TseitinExpr(boolean::Expr expr) {
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

  Minisat::Var traverse(boolean::Expr expr) {
    uint32_t v;
    boolean::Expr lhs, rhs;

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
    return Minisat::Var();
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

boolean::Expr nnf(boolean::Expr expr) {
  bool b;
  boolean::Expr lhs, rhs;

  if (expr.is_const()) {
    return expr;
  }

  if (expr.is_var()) {
    return expr;
  }

  if (expr.not_arg(lhs)) {
    auto arg = nnf(lhs);

    if (arg.get_value(b)) {
      return boolean::Expr::value(!b);
    }

    boolean::Expr inner;
    if (arg.not_arg(inner)) {
      return inner;
    }

    if (arg.and_args(lhs, rhs)) {
      return nnf(!lhs) || nnf(!rhs);
    }

    if (arg.or_args(lhs, rhs)) {
      return nnf(!lhs) && nnf(!rhs);
    }

    assert(arg.is_var());
    return !arg;
  }

  if (expr.and_args(lhs, rhs)) {
    auto lhsE = nnf(lhs);
    auto rhsE = nnf(rhs);

    if (lhsE == rhsE) {
      return lhsE;
    }

    if (lhsE.get_value(b)) {
      return b ? rhsE : lhsE;
    }

    if (rhsE.get_value(b)) {
      return b ? lhsE : rhsE;
    }

    return lhsE && rhsE;
  }

  if (expr.or_args(lhs, rhs)) {
    auto lhsE = nnf(lhs);
    auto rhsE = nnf(rhs);

    if (lhsE.get_value(b)) {
      return b ? lhsE : rhsE;
    }

    if (rhsE.get_value(b)) {
      return b ? rhsE : lhsE;
    }

    return lhsE || rhsE;
  }

  assert(false); // unreachable code
  return boolean::Expr();
}
/*
    Expr ll, lr, rl, rr;

    if (lhsE.or_args(ll, lr)) {
      if (rhsE.or_args(rl, rr)) {
        if (ll == rl) {
          return ll || (lr && rr);
        } else if (ll == rr) {
          return ll || (lr && rl);
        } else if (lr == rl) {
          return lr || (ll && rr);
        } else if (lr == rr) {
          return lr || (ll && rl);
        }
        return lhsE && rhsE;
      } else if (rhsE.and_args(rl, rr)) {
        if (ll == rl || ll == rr) {
          return lr && rhsE;
        } else if (lr == rl || lr == rr) {
          return ll && rhsE;
        }
        return lhsE && rhsE;
      }
    } !!! else if (lhsE.and_args(ll, lr)) {
      if (rhsE.or_args(rl, rr)) {
        if (ll == rl) {
          return ll || (lr && rr);
        } else if (ll == rr) {
          return ll || (lr && rl);
        } else if (lr == rl) {
          return lr || (ll && rr);
        } else if (lr == rr) {
          return lr || (ll && rl);
        }
        return lhsE && rhsE;
      } else if (rhsE.and_args(rl, rr)) {
        if (ll == rl || ll == rr) {
          return lr && rhsE;
        } else if (lr == rl || lr == rr) {
          return ll && rhsE;
        }
        return lhsE && rhsE;
      }
    }



    return lhsE && rhsE;
  }

*/

/**
 * Substitute expression with a const.
 * @param expr expression must be in NNF
 * @param search expression to search
 * @param replace expression to replace with
 * @returns new expression
 */
boolean::Expr subst(boolean::Expr expr, boolean::Expr search, boolean::Expr replace) {
  bool b;
  boolean::Expr r, lhs, rhs;

  if (expr == search) {
    return replace;
  }

  if (expr.is_const()) {
    return expr;
  }

  if (expr.is_var()) {
    return expr;
  }

  if (expr.not_arg(lhs)) {
    r = !subst(lhs, search, replace);
  } else if (expr.and_args(lhs, rhs)) {
    auto lhsE = subst(lhs, search, replace);
    auto rhsE = subst(rhs, search, replace);
    r = lhsE && rhsE;
  } else if (expr.or_args(lhs, rhs)) {
    auto lhsE = subst(lhs, search, replace);
    auto rhsE = subst(rhs, search, replace);
    r = lhsE || rhsE;
  } else {
    assert(false); // unreachable code
  }
  return nnf(r);
}

static bool prove(boolean::Expr expr) {
  return !sat(!expr);
}

static boolean::Expr implies(boolean::Expr u, boolean::Expr v) {
  return !u || v;
}

enum class ContOp {
  And, Or,
};

enum class ContHole {
  Lhs, Rhs,
};

struct ContCtx {
  ContOp op;
  ContHole hole;
  boolean::Expr self;
  boolean::Expr lhs;
  boolean::Expr rhs;
};

boolean::Expr simplify(boolean::Expr expr0) {
  constexpr size_t maxDepth = 2048;

  auto isLeaf = [] (boolean::Expr ex)
                { return
                    ex.is_var() ||
                    ex.is_not() ||
                    ex.is_const(); };

  std::array<ContCtx, maxDepth> stack;
  size_t esp {0};

  stack[esp].self = nnf(expr0);
  boolean::Expr& topExpr = stack[0].self;
  boolean::Expr lhs, rhs;

  while (true) {
    auto expr = stack[esp].self;
    if (isLeaf(expr)) {
      if (esp == 0) {
        return expr;
      }
      // leaf found, replace it with true&false to produce
      // new candidates
      boolean::Expr neg{boolean::Expr::value(false)};
      boolean::Expr pos{boolean::Expr::value(true)};
      boolean::Expr subst;
      for (size_t ix = esp - 1; ; --ix) {
        auto cont = stack[ix];
        auto arg = cont.hole == ContHole::Lhs ? cont.rhs : cont.lhs;
        switch (cont.op) {
        case ContOp::Or:
          pos = pos || arg;
          neg = neg || arg;
          break;
        case ContOp::And:
          pos = pos && arg;
          neg = neg && arg;
          break;
        }

        if (ix == 0) {
          break;
        }
      }

      neg = nnf(neg);
      pos = nnf(pos);

      /**
         Definition 5. (Redundancy) We say a leaf L is non-constraining in formula
         φ if φ+ (L) ⇒ φ and non-relaxing if φ ⇒ φ− (L). Leaf L is redundant if L is
         either non-constraining or non-relaxing.
      */

      if (prove(implies(pos,topExpr))) {
        // pos is new reduced formula
        esp = 0;
        topExpr = pos;
        continue;
      }
      if (prove(implies(topExpr, neg))) {
        // neg is new reduced formula
        esp = 0;
        topExpr = neg;
        continue;
      }

      // skip to next alternative
      assert(esp > 0);
      esp -= 1;

      while (stack[esp].hole == ContHole::Rhs) {
        if (esp == 0) {
          // that is it, nothing can be done anymore
          return topExpr;
        }
        // rollback to upper level
        esp -= 1;
      }

      assert(esp >= 0);
      assert(stack[esp].hole == ContHole::Lhs);
      stack[esp].hole = ContHole::Rhs;
      ++esp;
      stack[esp].self = stack[esp - 1].rhs;
      continue;
    }
    if (expr.and_args(lhs, rhs)) {
      stack[esp].op = ContOp::And;
      stack[esp].hole = ContHole::Lhs;
      stack[esp].lhs = lhs;
      stack[esp].rhs = rhs;
      ++esp;
      stack[esp].self = lhs;
      continue;
    }
    if (expr.or_args(lhs, rhs)) {
      stack[esp].op = ContOp::Or;
      stack[esp].hole = ContHole::Lhs;
      stack[esp].lhs = lhs;
      stack[esp].rhs = rhs;
      ++esp;
      stack[esp].self = lhs;
      continue;
    }
  }
}

bool sat(BoolExpr& expr) {
  auto simplifiedExpr = Simplify(expr).getResult();
  return TseitinBoolExpr(*simplifiedExpr).isSatisfiable();
}

bool sat(boolean::Expr expr) {
  auto simplifiedExpr = nnf(expr);
  return TseitinExpr(simplifiedExpr).isSatisfiable();
}

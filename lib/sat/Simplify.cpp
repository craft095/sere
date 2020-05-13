#include "sat/Transform.hpp"
#include "sat/Simplify.hpp"
#include "sat/Sat.hpp"

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

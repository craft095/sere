#include "sat/Transform.hpp"
#include "boolean/Expr.hpp"

/**
 * Convert to NNF
 *
 * The result:
 * - either TRUE
 * - or FALSE
 * - or it does not contain neither TRUE nor FALSE
 *   and NEGATION is only applied to variable
 */
boolean::Expr nnf0(boolean::Expr expr) ;

boolean::Expr nnf(boolean::Expr expr) {
  if (expr.is_const()) {
    return expr;
  }

  if (expr.is_var()) {
    return expr;
  }

  return expr.query_and_update_func(boolean::F_NNF, nnf0);
}

boolean::Expr nnf0(boolean::Expr expr) {
  bool b;
  boolean::Expr lhs, rhs;

  // if (expr.is_const()) {
  //   return expr;
  // }

  // if (expr.is_var()) {
  //   return expr;
  // }

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

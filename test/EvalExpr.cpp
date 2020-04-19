#include "test/Letter.hpp"
#include "boolean/Expr.hpp"

bool evalBool(boolean::Expr expr, const rt::Names& letter) {
  bool tf;
  uint32_t v;
  boolean::Expr lhs, rhs;

  if (expr.get_value(tf)) {
    return tf;
  }
  if (expr.get_var(v)) {
    return letter.test(v);
  }
  if (expr.not_arg(lhs)) {
    return !evalBool(lhs, letter);
  }
  if (expr.and_args(lhs, rhs)) {
    return evalBool(lhs, letter) && evalBool(rhs, letter);
  }
  if (expr.or_args(lhs, rhs)) {
    return evalBool(lhs, letter) || evalBool(rhs, letter);
  }
  assert(false); // unreachable code
}

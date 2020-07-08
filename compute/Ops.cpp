#include "Ops.hpp"
#include "Ast.hpp"

#include "ast/Located.hpp"

#include <vector>

namespace compute {
  static std::vector<Ident::Ptr> builtinOps;

  Ident::Ptr lookupBuiltinOp(OpId::Ix ix) {
    return builtinOps.at(ix);
  }

  static void newBuiltinOp(OpId::Ix ix, const char* name) {
    Ident::Ptr i { std::make_shared<Ident> (RE_LOC, name) };
    builtinOps.at(ix) = i;
  }

  void fillBuiltinOps() {
    if (builtinOps.size() != 0) {
      return;
    } else {
      builtinOps.resize(OpId::OP_COUNT);
    }
    newBuiltinOp(OpId::MATH_NEG, "__math_neg");
    newBuiltinOp(OpId::BOOL_NOT, "__bool_not");
    newBuiltinOp(OpId::SERE_COMPLEMENT, "__sere_complement");
    newBuiltinOp(OpId::SERE_KLEENESTAR, "__sere_kleenestar");
    newBuiltinOp(OpId::SERE_KLEENEPLUS, "__sere_kleeneplus");

    newBuiltinOp(OpId::MATH_ADD, "__math_add");
    newBuiltinOp(OpId::MATH_SUB, "__math_sub");
    newBuiltinOp(OpId::MATH_DIV, "__math_div");
    newBuiltinOp(OpId::MATH_MUL, "__math_mul");
    newBuiltinOp(OpId::BOOL_AND, "__bool_and");
    newBuiltinOp(OpId::BOOL_OR, "__bool_or");
    newBuiltinOp(OpId::BOOL_EQ, "__bool_eq");
    newBuiltinOp(OpId::BOOL_NE, "__bool_ne");
    newBuiltinOp(OpId::BOOL_LT, "__bool_lt");
    newBuiltinOp(OpId::BOOL_LE, "__bool_le");
    newBuiltinOp(OpId::BOOL_GT, "__bool_gt");
    newBuiltinOp(OpId::BOOL_GE, "__bool_ge");
    newBuiltinOp(OpId::SERE_INTERSECT, "__sere_intersect");
    newBuiltinOp(OpId::SERE_UNION, "__sere_union");
    newBuiltinOp(OpId::SERE_CONCAT, "__sere_concat");
    newBuiltinOp(OpId::SERE_FUSION, "__sere_fusion");
  }

} // namespace compute

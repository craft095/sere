#ifndef COMPUTE_OPS_HPP
#define COMPUTE_OPS_HPP

#include "Ast.hpp"

namespace compute {

  struct OpId {
    typedef enum {
          MATH_NEG,
          BOOL_NOT,
          SERE_COMPLEMENT,
          SERE_KLEENESTAR,
          SERE_KLEENEPLUS,

          MATH_ADD,
          MATH_SUB,
          MATH_DIV,
          MATH_MUL,
          BOOL_AND,
          BOOL_OR,
          BOOL_EQ,
          BOOL_NE,
          BOOL_LT,
          BOOL_LE,
          BOOL_GT,
          BOOL_GE,
          SERE_INTERSECT,
          SERE_UNION,
          SERE_CONCAT,
          SERE_FUSION,

          OP_COUNT,
    } Ix;
  };

  extern Ident::Ptr lookupBuiltinOp(OpId::Ix ix);
  extern void fillBuiltinOps();

} // namespace compute

#endif //COMPUTE_OPS_HPP

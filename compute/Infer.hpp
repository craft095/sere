#ifndef COMPUTE_INFER_HPP
#define COMPUTE_INFER_HPP

#include "Typed.hpp"
#include "Ast.hpp"

#include <string>
#include <vector>
#include <memory>

namespace compute {
  class NameContext;

  extern TypedNode::Ptr infer(const NameContext& context, Root::Ptr root);
  extern TypedNode::Ptr inferExpr(const NameContext& context, Expression::Ptr expr);
} // namespace compute

#endif // COMPUTE_INFER_HPP

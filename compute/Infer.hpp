#ifndef COMPUTE_INFER_HPP
#define COMPUTE_INFER_HPP

#include "Typed.hpp"
#include "Ast.hpp"

#include <string>
#include <vector>
#include <memory>

namespace parser {
  class ParseResult;
}

namespace compute {
  class NameContext;

  extern TypedNode::Ptr infer(const NameContext& context, Root::Ptr root);
  extern TypedNode::Ptr inferExpr(const NameContext& context, Expression::Ptr expr);
  extern void toSere(Root::Ptr root, TypedNode::Ptr typed, parser::ParseResult& result);
} // namespace compute

#endif // COMPUTE_INFER_HPP

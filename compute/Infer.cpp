#include "Infer.hpp"
#include "NameContext.hpp"
#include "Typed.hpp"
#include "Ast.hpp"

namespace compute {
  class Typer : public ExpressionVisitor {
    TypedNode::Ptr result;
    const NameContext& context;

  public:
    Typer(const NameContext& context_) : context(context_) {}

    TypedNode::Ptr getResult() const { return result; }

    void visit(StringLit* v) override;
    void visit(FloatLit* v) override;
    void visit(IntLit* v) override;
    void visit(BoolLit* v) override;
    void visit(SereLit* v) override;
    void visit(NameRef* v) override;
    void visit(FuncCall* v) override;
  };

  void Typer::visit(StringLit* v) {
    result = Scalar::create(v, { TypeId::String });
  }

  void Typer::visit(FloatLit* v) {
    result = Scalar::create(v, { TypeId::Float });
  }

  void Typer::visit(IntLit* v) {
    TypeIds args;
    if (v->getValue() >= 0) {
      if (v->getValue() <= INT8_MAX) {
        args.insert(TypeId::UInt8);
      }
      if (v->getValue() <= INT16_MAX) {
        args.insert(TypeId::UInt16);
      }
      if (v->getValue() <= INT32_MAX) {
        args.insert(TypeId::UInt32);
      }
      if (v->getValue() <= INT64_MAX) {
        args.insert(TypeId::UInt64);
      }
      if (v->getValue() < INT8_MAX) {
        args.insert(TypeId::SInt8);
      }
      if (v->getValue() < INT16_MAX) {
        args.insert(TypeId::SInt16);
      }
      if (v->getValue() < INT32_MAX) {
        args.insert(TypeId::SInt32);
      }
      if (v->getValue() < INT64_MAX) {
        args.insert(TypeId::SInt64);
      }
    } else {
      if (v->getValue() >= INT8_MIN) {
        args.insert(TypeId::SInt8);
      }
      if (v->getValue() >= INT16_MIN) {
        args.insert(TypeId::SInt16);
      }
      if (v->getValue() >= INT32_MIN) {
        args.insert(TypeId::SInt32);
      }
      if (v->getValue() >= INT64_MIN) {
        args.insert(TypeId::SInt64);
      }
    }
    result = Scalar::create(v, args);
  }

  void Typer::visit(BoolLit* v) {
    result = Scalar::create(v, {TypeId::Bool, TypeId::Sere});
  }

  void Typer::visit(SereLit* v) {
    result = Scalar::create(v, {TypeId::Sere});
  }

  void Typer::visit(NameRef* v) {
    TypedNode::Ptr node = context.lookupScalar(v->getName()->getName());

    assert(node); // TODO: emit "name not found"

    // make a copy for futher refining
    result = node->clone();
  }

  void Typer::visit(FuncCall* v) {
    Func::Ptr node = context.lookupFunc(v->getName()->getName());

    assert(node); // TODO: emit "name not found"

    Func::Ptr func = node->clone();

    TypedNodes args;
    for (auto arg : v->getArgs()) {
      arg->accept(*this);
      args.push_back(result);
    }

    result = Apply::create(v, func, args);
  }

  /**
   * Get all types which can represent given type
   */
  TypeIds generalize(TypeId typ) {
    TypeIds ids{typ};
    switch (typ) {
    case TypeId::None:
      break;
    case TypeId::Bool:
      ids.insert(TypeId::Sere);
      break;
    case TypeId::Sere:
      break;
    case TypeId::UInt8:
      ids.insert({
                  TypeId::UInt16, TypeId::UInt32, TypeId::UInt64,
                  TypeId::SInt16, TypeId::SInt32, TypeId::SInt64, TypeId::Float
                 });      break;
    case TypeId::UInt16:
      ids.insert({
                  TypeId::UInt32, TypeId::UInt64,
                  TypeId::SInt32, TypeId::SInt64, TypeId::Float,
                 });
      break;
    case TypeId::UInt32:
      ids.insert({
                  TypeId::UInt64,
                  TypeId::SInt64, TypeId::Float,
                 });
      break;
    case TypeId::UInt64:
      break;
    case TypeId::SInt8:
      ids.insert({
                  TypeId::SInt16, TypeId::SInt32, TypeId::SInt64, TypeId::Float,
                 });
      break;
    case TypeId::SInt16:
      ids.insert({
                  TypeId::SInt32, TypeId::SInt64, TypeId::Float,
                 });
      break;
    case TypeId::SInt32:
      ids.insert({
                  TypeId::SInt64, TypeId::Float,
                 });
      break;
    case TypeId::SInt64:
      break;
    case TypeId::Float:
      break;
    case TypeId::String:
      break;
    case TypeId::Time:
      break;
    }
    return ids;
  }

  TypedNode::Ptr infer(const NameContext& context0, Root::Ptr root) {
    NameContext context(context0);
    for (auto arg : root->getArgDecls()) {
      context.insertScalar(arg->getName()->getName(),
                           Scalar::create(arg.get(), { generalize(arg->getTypeId()) }));
    }

    for (auto decl : root->getLetDecls()) {

    }

    return inferExpr(context, root->getExpression());
  }

  TypedNode::Ptr inferExpr(const NameContext& context, Expression::Ptr expr) {
    Typer typer(context);
    expr->accept(typer);
    return typer.getResult();
  }

} // namespace compute

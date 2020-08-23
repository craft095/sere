#include "Infer.hpp"
#include "Error.hpp"
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
    const Ident::Name& name = v->getName()->getName();
    if (context.lookupFunc(name)) {
      throw ScalarExpected(v->getLoc(), name);
    }
    TypedNode::Ptr node = context.lookupScalar(name);

    if (!node) {
      throw NameNotFound(v->getLoc(), name);
    }

    result = node;
  }

  void Typer::visit(FuncCall* v) {
    const Ident::Name& name = v->getName()->getName();
    if (context.lookupScalar(name)) {
      throw FuncExpected(v->getLoc(), name);
    }
    Func::Ptr node = context.lookupFunc(name);

    if (!node) {
      throw NameNotFound(v->getLoc(), name);
    }

    Func::Ptr func = node;

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
      context.insertScalar(arg->getNameRef()->getName()->getName(),
                           Scalar::create(arg->getNameRef().get(),
                                          { generalize(arg->getTypeId()) }));
    }

    for (auto decl : root->getLetDecls()) {
      Ident::Name name = decl->getName()->getName();
      if (decl->getArgs().empty()) {
        // scalar
        context.insertScalar(name,
                             inferExpr(context, decl->getBody()));
      } else {
        // function
#if 0
        MonoNameContext locals(&context);
        for (auto arg : decl->getArgs()) {
          locals.insertScalar(arg->getName(), Scalar::create(arg.get(), anyType));
        }
        context.insertFunc(name,
                           inferExpr(locals, decl->getBody()));
#else
        assert(false); // not implemented
#endif
      }
    }

    return inferExpr(context, root->getExpression());
  }

  TypedNode::Ptr inferExpr(const NameContext& context, Expression::Ptr expr) {
    Typer typer(context);
    expr->accept(typer);
    return typer.getResult();
  }

#if 0
  class Atomics {
  public:
    typedef std::map<Ident::Name, size_t> Vars;
    typedef std::map<Ident::Name, TypedNode*> Exprs;

    VarName addAtomic(Ident::Ptr id, TypedNode* expr) {
      const Ident::Name& name { id->getName() };
      auto i = vars.insert(Vars::value_type {name, vars.size()});
      if (i.second) {
        exprs[name] = expr;
      }
      return VarName { i.first->second };
    }

  private:
    Vars vars;
    Exprs expr;
  };

  class ToBool : public ExpressionVisitor {
    Atomics& atomics;
    Scalar* scalar;
    BoolExpr::Ptr result;

  public:
    ToBool(Atomics& atomics_, Scalar* scalar_)
      : atomics(atomics_), scalar(scalar_) {}

    TypedNode::Ptr getResult() const { return result; }

    void visit(StringLit* v) override { assert(false); }
    void visit(FloatLit* v) override { assert(false); }
    void visit(IntLit* v) override { assert(false); }
    void visit(BoolLit* v) override {
      result = std::make_shared<BoolValue>(v->getLoc(), v->getValue());
    }
    void visit(SereLit* v) override { assert(false); }
    void visit(NameRef* v) override {
      assert(scalar->getFinalType() == TypeId::Bool);
      VarName var { atomics.addVarName(v->getName, scalar) };
      result = std::make_shared<Variable>(v->getLoc(), var);
    }
    void visit(FuncCall* v) override {}
  };

  class ToSere : public ExpressionVisitor {
    TypedNode::Ptr result;
    const NameContext& context;

  public:
    ToSere(const NameContext& context_) : context(context_) {}

    TypedNode::Ptr getResult() const { return result; }

    void visit(StringLit* v) override;
    void visit(FloatLit* v) override;
    void visit(IntLit* v) override;
    void visit(BoolLit* v) override;
    void visit(SereLit* v) override;
    void visit(NameRef* v) override;
    void visit(FuncCall* v) override;
  };

  void ToSere::visit(StringLit* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(FloatLit* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(IntLit* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(BoolLit* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(SereLit* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(NameRef* v) {
    assert(false); // not implemented
  }

  void ToSere::visit(FuncCall* v) {
    assert(false); // not implemented
  }
  */

  struct ParseResult {
    Ptr<SereExpr> expr;
    AtomicNameMap vars;
  };

  void toSere(TypedNode::Ptr typed, parser::ParseResult& result) {
    if (auto scalar = dynamic_cast<Scalar*>(typed.get())) {
      TypeId typeId = scalar->getTypeIds().front();

      switch (typeId) {
      case TypeId::Bool:

      }
      ToSere(scalar)
    }
  }
#endif
} // namespace compute

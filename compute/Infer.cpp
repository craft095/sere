#include "Infer.hpp"
#include "Error.hpp"
#include "NameContext.hpp"
#include "Typed.hpp"
#include "Ast.hpp"

#include "ast/Parser.hpp"
#include "ast/SereExpr.hpp"
#include "ast/BoolExpr.hpp"

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

#if 1
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

    void fillVars(parser::AtomicNameMap& vs) const {
      vs = vars;
    }

  private:
    Vars vars;
    Exprs exprs;
  };

  class ToSereVisitor : public TypedNodeVisitor {
    Atomics& atomics;
    const ArgDecls& argDecls;
    Ptr<SereExpr> sereExpr;
    Ptr<BoolExpr> boolExpr;

  public:
    ToSereVisitor(Atomics& atomics_, const ArgDecls& argDecls_)
      : atomics(atomics_), argDecls(argDecls_) {}

    void resolveName(Scalar& node, Ident::Ptr name) {
      // name is taken from either args or it should be calculated
      // at the moment, only first case is handled

      // find name in arg decls
      for (auto& argDecl : argDecls) {
        if (argDecl->getNameRef()->getName()->getName() == name->getName()) {
          assert(argDecl->getTypeId() == TypeId::Bool); // TODO: the only supported case

          VarName var { atomics.addAtomic(name, &node) };
          boolExpr = std::make_shared<Variable>(node.getLoc(), var);
          sereExpr = nullptr;
          return;
        }
      }

      assert(false); // TODO: add name calculation here
    }

    void visit(Scalar& node) override {
      boolExpr = nullptr;
      sereExpr = nullptr;

      if (auto value = dynamic_cast<const BoolLit*>(node.getNode())) {
        boolExpr = std::make_shared<BoolValue>(node.getLoc(), value->getValue());
      } else if (auto value = dynamic_cast<const SereLit*>(node.getNode())) {
        assert(value->getValue() == SereLiteral::EPS);
        sereExpr = std::make_shared<SereEmpty>(value->getLoc());
      } else if (auto nameValue = dynamic_cast<const NameRef*>(node.getNode())) {
        resolveName(node, nameValue->getName());
      } else {
        assert(false); // not implemented
      }
    }

    Ptr<BoolExpr> getBool() const {
      assert(boolExpr != nullptr);
      return boolExpr;
    }

    Ptr<SereExpr> getSere() const {
      assert(sereExpr != nullptr || boolExpr != nullptr);

      if (sereExpr != nullptr) {
        return sereExpr;
      }

      return std::make_shared<SereBool>(boolExpr->getLoc(), boolExpr);
    }

    template <typename T>
    void boolBinary(const Located& loc, const TypedNodes& args) {
        assert(args.size() == 2);

        args[0]->accept(*this);
        auto lhs = getBool();
        args[1]->accept(*this);
        auto rhs = getBool();

        boolExpr = std::make_shared<T>(loc, lhs, rhs);
        sereExpr = nullptr;
    }

    template <typename T>
    void sereBinary(const Located& loc, const TypedNodes& args) {
        assert(args.size() == 2);

        args[0]->accept(*this);
        auto lhs = getSere();
        args[1]->accept(*this);
        auto rhs = getSere();

        sereExpr = std::make_shared<T>(loc, lhs, rhs);
        boolExpr = nullptr;
    }

    template <typename T>
    void boolUnary(const Located& loc, const TypedNodes& args) {
        assert(args.size() == 1);

        args[0]->accept(*this);
        auto arg = getBool();

        boolExpr = std::make_shared<T>(loc, arg);
        sereExpr = nullptr;
    }

    template <typename T>
    void sereUnary(const Located& loc, const TypedNodes& args) {
        assert(args.size() == 1);

        args[0]->accept(*this);
        auto arg = getSere();

        sereExpr = std::make_shared<T>(loc, arg);
        boolExpr = nullptr;
    }

    void visit(Apply& node) override {
      Func::Ptr func { node.getFunc() };

      auto call = dynamic_cast<const FuncCall*>(node.getNode());

      assert(call != nullptr);

      Ident::Name name = call->getName()->getName();

      // TODO: there must be calculation code instead of assert
      // assert(context.isFuncTransparent(name));

      auto const& args = node.getArgs();

      boolExpr = nullptr;
      sereExpr = nullptr;

      if (name == "__bool_and") {
        boolBinary<BoolAnd>(node.getLoc(), args);
      } else if (name == "__bool_or") {
        boolBinary<BoolOr>(node.getLoc(), args);
      } else if (name == "__bool_not") {
        boolUnary<BoolNot>(node.getLoc(), args);
      } else if (name == "__sere_complement") {
        sereUnary<Complement>(node.getLoc(), args);
      } else if (name == "__sere_kleenestar") {
        sereUnary<KleeneStar>(node.getLoc(), args);
      } else if (name == "__sere_kleeneplus") {
        sereUnary<KleenePlus>(node.getLoc(), args);
      } else if (name == "__sere_intersect") {
        sereBinary<Intersect>(node.getLoc(), args);
      } else if (name == "__sere_union") {
        sereBinary<Union>(node.getLoc(), args);
      } else if (name == "__sere_concat") {
        sereBinary<Concat>(node.getLoc(), args);
      } else if (name == "__sere_fusion") {
        sereBinary<Fusion>(node.getLoc(), args);
      } else {
        assert(false); // unreachable code
      }
    }
  };

  void toSere(Root::Ptr root, TypedNode::Ptr typed, parser::ParseResult& result) {
    Atomics atomics;
    ToSereVisitor visitor(atomics, root->getArgDecls());

    typed->accept(visitor);

    result.expr = visitor.getSere();
    atomics.fillVars(result.vars);
  }
#endif
} // namespace compute

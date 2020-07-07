#include "Typed.hpp"

#include <nlohmann/json.hpp>

namespace compute {
  const String TypedNode::pretty() const {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(*this).dump(spaces) << std::endl;
    return s.str();
  }

  void to_json(json& j, const FuncType& ft) {
    j = json {
      {"node",   "FuncType" },
      {"args",   ft.args },
      {"result", ft.result },
    };
  }

  void to_json(json& j, const TypedNode& a) {
    a.to_json(j);
  }

  void to_json(json& j, const Func& a) {
    a.to_json(j);
  }

  void Scalar::to_json(json& j) const {
    j = json {
      {"node",     "Scalar" },
      {"typeIds",  typeIds },
    };
  }

  void Apply::to_json(json& j) const {
    j = json {
      {"node",  "Apply" },
      {"func",  func },
      {"args",  args },
    };
  }

  void Func::to_json(json& j) const {
    j = json {
      {"node",      "Func" },
      {"funcTypes", funcTypes },
    };
  }

  TypedNode::Ptr NameContext::lookupScalar(const Ident::Name& name) const {
    auto i = scalarContext.find(name);
    return i == scalarContext.end() ? nullptr : i->second;
  }

  Func::Ptr NameContext::lookupFunc(const Ident::Name& name) const {
    auto i = funcContext.find(name);
    return i == funcContext.end() ? nullptr : i->second;
  }

  void NameContext::insertScalar(const Ident::Name& name, TypedNode::Ptr typed) {
    assert(lookupFunc(name) == nullptr);
    assert(lookupScalar(name) == nullptr);
    scalarContext[name] = typed;
  }

  void NameContext::insertFunc(const Ident::Name& name, Func::Ptr func) {
    assert(lookupFunc(name) == nullptr);
    assert(lookupScalar(name) == nullptr);
    funcContext[name] = func;
  }

  void NameContext::to_json(json& j) const {
    j = json {
      {"node",    "NameContext" },
      {"funcs",   funcContext },
      {"scalars", scalarContext },
    };
  }

  void Typer::visit(StringLit* v) {
    result = Scalar::create({ TypeId::String });
  }

  void Typer::visit(FloatLit* v) {
    result = Scalar::create({ TypeId::Float });
  }

  void Typer::visit(IntLit* v) {
    TypeIds args;
    if (v->getValue() < ~(uint8_t)0) {
      args.push_back(TypeId::SInt8);
    }
    if (v->getValue() <= ~(uint8_t)0) {
      args.push_back(TypeId::UInt8);
    }
    if (v->getValue() < ~(uint64_t)0) {
      args.push_back(TypeId::SInt16);
    }
    if (v->getValue() <= ~(uint16_t)0) {
      args.push_back(TypeId::UInt16);
    }
    if (v->getValue() < ~(uint32_t)0) {
      args.push_back(TypeId::SInt32);
    }
    if (v->getValue() <= ~(uint32_t)0) {
      args.push_back(TypeId::UInt32);
    }
    if (v->getValue() < ~(uint64_t)0) {
      args.push_back(TypeId::SInt64);
    }
    if (v->getValue() <= ~(uint64_t)0) {
      args.push_back(TypeId::UInt64);
    }

    result = Scalar::create(args);
  }

  void Typer::visit(BoolLit* v) {
    result = Scalar::create({TypeId::Bool, TypeId::Sere});
  }

  void Typer::visit(SereLit* v) {
    result = Scalar::create({TypeId::Sere});
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

    result = Apply::create(func, args);
  }

  TypedNode::Ptr inferTypes(const NameContext& context, Expression::Ptr expr) {
    Typer typer(context);
    expr->accept(typer);
    return typer.getResult();
  }

} // namespace compute

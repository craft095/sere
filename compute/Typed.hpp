#ifndef COMPUTE_TYPED_HPP
#define COMPUTE_TYPED_HPP

#include "ast/Common.hpp"
#include "ast/Located.hpp"

#include <string>
#include <vector>
#include <memory>

/**
   Type system.
 */

namespace compute {
  // class TypeDesc {
  //   typedef std::string VarName;
  //   typedef std::vector<TypeAtomicRef> VarConstraint;
  //   typedef std::map<VarName, VarConstraint> VarConstraints;
  //   typedef std::vector<VarName> Tuple;


  //   VarConstraints context;
  //   Tuple args;
  //   VarName ret;



  // };


  typedef std::vector<TypeId> TypeIds;

  struct FuncType {
    TypeIds args;
    TypeId result;
  };

  typedef std::vector<FuncType> FuncTypes;

  class TypedNode {
  public:
    typedef std::shared_ptr<TypedNode> Ptr;

    virtual ~TypedNode() {}

    virtual const TypeIds& getTypeIds() const = 0;
    virtual Ptr clone() const = 0;
    virtual void to_json(json& j) const = 0;
  };

  typedef std::vector<TypedNode> TypedNodes;

  class Scalar : public TypedNode {
  public:
    typedef std::shared_ptr<Scalar> Ptr;

    static Ptr create(TypeIds typs) {
      return std::make_shared<Scalar>(typs);
    }

    TypedNode(TypeIds typs) : typeIds(typs) {}
    const TypeIds& getTypeIds() const override { return typeIds; }

    Ptr clone() const override {
      return create(typeIds);
    }

    void to_json(json& j) const override;

  private:
    TypeIds typeIds;
  };

  class Func /*: public TypedNode*/ {
  public:
    typedef std::shared_ptr<Func> Ptr;

    static Ptr create(FuncTypes typs) {
      return std::make_shared<Func>(typs);
    }

    Func(const FuncTypes& ft) : funcTypes(ft) {}

    Ptr clone() const override {
      return create(funcTypes);
    }

    void to_json(json& j) const;
  private:
    FuncTypes funcTypes;
  };

  class Apply : public TypedNode {
  public:
    typedef std::shared_ptr<Apply> Ptr;

    static Ptr create(Func::Ptr func, TypedNodes& args) {
      return std::make_shared<Apply>(func, args);
    }

    Apply(Func::Ptr f, TypedNodes& as) : func(f), args(as) {}

    Ptr clone() const override {
      Args args1;
      for (auto arg : args) {
        args1.push_back(arg->clone());
      }
      return create(func->clone(), args1);
    }

    void to_json(json& j) const override;

  private:
    Func::Ptr func;
    TypedNodes args;
  };

  class NameContext {
  public:
    /**
       Find name in scalar context
     */
    TypedNode::Ptr lookupScalar(const Ident::Name& name) const;
    /**
       Find name in functional context
     */
    Func::Ptr lookupFunc(const Ident::Name& name) const;
    /**
       Insert name into scalar context. Name must be new one.
     */
    void insertScalar(const Ident::Name& name, TypedNode::Ptr typed);
    /**
       Insert name into functional context. Name must be new one.
     */
    void insertFunc(const Ident::Name& name, Func::Ptr func);

    void to_json(json& j) const;

  private:
    typedef std::map<Ident::Name, TypedNode::Ptr> ScalarContext;
    typedef std::map<Ident::Name, FuncNode::Ptr> FuncContext;
    FuncContext funcContext;
    ScalarContext ScalarContext;
  };

  class Typer : publicExpressionVisitor {
    TypedNode::Ptr result;
    const NameContext& context;

  public:
    Typer(const NameContext& context_) : context(context_) {}

    void visit(StringLit* v) override;
    void visit(FloatLit* v) override;
    void visit(IntLit* v) override;
    void visit(BoolLit* v) override;
    void visit(SereLit* v) override;
    void visit(NameRef* v) override;
    void visit(FuncCall* v) override;
  };

  extern void to_json(json& j, const TypedNode& a);
  extern void to_json(json& j, const Func& a);
} // namespace compute

#endif // COMPUTE_TYPED_HPP

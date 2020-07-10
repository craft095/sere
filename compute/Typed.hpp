#ifndef COMPUTE_TYPED_HPP
#define COMPUTE_TYPED_HPP

#include "Ast.hpp"
#include "ast/Common.hpp"
#include "ast/Located.hpp"

#include <set>
#include <string>
#include <vector>
#include <memory>

/**
   Type system.
 */

namespace compute {
  typedef std::set<TypeId> TypeIds;

  struct FuncType {
    std::vector<TypeId> args;
    TypeId result;

    bool operator<(const FuncType& u) const {
      return result < u.result || (result == u.result && args < u.args);
    }
  };

  typedef std::set<FuncType> FuncTypes;

  /*class ScalarTypeMismatch : public Error {
  public:
    ScalarTypeMismatch(const Located& loc,
                       const TypeIds& actual,
                       const TypeIds& expected);
  };

  class FuncTypeMismatch : public Error {
  public:
    FuncTypeMismatch(const Located& loc,
                     const FuncTypeIds& actual,
                     const FuncTypeIds& expected);
  };

  class NameNotFound : public Error {
  public:
    NameNotFound(Ident::Ptr id);
  };

  class NameMisuse : public Error {
  public:
    NameMisuse(Ident::Ptr id, const std::string& descr);
    };*/

  class TypedNode {
  public:
    typedef std::shared_ptr<TypedNode> Ptr;

    virtual ~TypedNode() {}

    const String pretty() const;

    virtual void constrain(const TypeIds& typs) = 0;
    virtual const TypeIds& getTypeIds() const = 0;
    virtual Ptr clone() const = 0;
    virtual void to_json(json& j) const = 0;
  };

  typedef std::vector<TypedNode::Ptr> TypedNodes;

  class Scalar : public TypedNode {
  public:
    typedef std::shared_ptr<Scalar> Ptr;

    static Ptr create(const TypeIds& typs) {
      return std::make_shared<Scalar>(typs);
    }

    Scalar(const TypeIds& typs) : typeIds(typs) {}
    void constrain(const TypeIds& typs) override;
    const TypeIds& getTypeIds() const override { return typeIds; }

    TypedNode::Ptr clone() const override {
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

    Ptr clone() const {
      return create(funcTypes);
    }

    void to_json(json& j) const;
    const FuncTypes& getTypes() const { return funcTypes; }
    void constrain(const FuncTypes& typs);

  private:
    FuncTypes funcTypes;
  };

  class Apply : public TypedNode {
  public:
    typedef std::shared_ptr<Apply> Ptr;

    static Ptr create(Func::Ptr func, TypedNodes& args) {
      return std::make_shared<Apply>(func, args);
    }

    Apply(Func::Ptr f, TypedNodes& as);

    TypedNode::Ptr clone() const override {
      TypedNodes args1;
      for (auto arg : args) {
        args1.push_back(arg->clone());
      }
      return create(func->clone(), args1);
    }

    void to_json(json& j) const override;

    const TypeIds& getTypeIds() const override { return typeIds; }
    void constrain(const TypeIds& typs) override;

  private:
    FuncTypes funcTypes;
    TypeIds typeIds;
    Func::Ptr func;
    TypedNodes args;
  };

  extern void to_json(json& j, const TypedNode& a);
  extern void to_json(json& j, const Func& a);
} // namespace compute

#endif // COMPUTE_TYPED_HPP

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

  extern TypeIds anyType;

  struct FuncType {
    std::vector<TypeId> args;
    TypeId result;

    bool operator<(const FuncType& u) const {
      return result < u.result || (result == u.result && args < u.args);
    }
  };

  typedef std::set<FuncType> FuncTypes;

  class TypedNode {
  public:
    typedef std::shared_ptr<TypedNode> Ptr;

    /**
     * Create TypedNode out of AST node
     * @param node AST node
     */
    TypedNode(const Node* node_) : node(node_) {}
    virtual ~TypedNode() {}

    const String pretty() const;

    const Node* getNode() const { return node; }

    virtual void constrain(const TypeIds& typs) = 0;
    virtual const TypeIds& getTypeIds() const = 0;
    virtual Ptr clone() const = 0;
    virtual void to_json(json& j) const = 0;
  private:
    const Node* node;
  };

  typedef std::vector<TypedNode::Ptr> TypedNodes;

  class Scalar : public TypedNode {
  public:
    typedef std::shared_ptr<Scalar> Ptr;

    static Ptr create(const Node* node, const TypeIds& typs) {
      return std::make_shared<Scalar>(node, typs);
    }

    Scalar(const Node* node, const TypeIds& typs)
      : TypedNode(node), typeIds(typs) {}
    void constrain(const TypeIds& typs) override;
    const TypeIds& getTypeIds() const override { return typeIds; }

    TypedNode::Ptr clone() const override {
      return create(getNode(), typeIds);
    }

    void to_json(json& j) const override;

  private:
    TypeIds typeIds;
  };

  class Func /*: public TypedNode*/ {
  public:
    typedef std::shared_ptr<Func> Ptr;

    static Ptr create(const Node* node, FuncTypes typs) {
      return std::make_shared<Func>(node, typs);
    }

    Func(const Node* node_, const FuncTypes& ft)
      : node(node_), funcTypes(ft) {}

    Ptr clone() const {
      return create(getNode(), funcTypes);
    }

    const Node* getNode() const { return node; }
    void to_json(json& j) const;
    const FuncTypes& getTypes() const { return funcTypes; }
    void constrain(const FuncTypes& typs);

  private:
    const Node* node;
    FuncTypes funcTypes;
  };

  class Apply : public TypedNode {
  public:
    typedef std::shared_ptr<Apply> Ptr;

    static Ptr create(const Node* node, Func::Ptr func, TypedNodes& args) {
      return std::make_shared<Apply>(node, func, args);
    }

    Apply(const Node* node, Func::Ptr f, TypedNodes& as);

    TypedNode::Ptr clone() const override {
      TypedNodes args1;
      for (auto arg : args) {
        args1.push_back(arg->clone());
      }
      return create(getNode(), func->clone(), args1);
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

  template <typename T>
  const String pretty(const T& v) {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(v).dump(spaces) << std::endl;
    return s.str();
  }

  extern void to_json(json& j, const FuncType& ft);
  extern void to_json(json& j, const TypedNode& a);
  extern void to_json(json& j, const Func& a);
} // namespace compute

#endif // COMPUTE_TYPED_HPP

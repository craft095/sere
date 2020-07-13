#ifndef COMPUTE_NAMECONTEXT_HPP
#define COMPUTE_NAMECONTEXT_HPP

#include "Typed.hpp"

#include <map>

namespace compute {
  class NameContext {
  public:
    NameContext(const NameContext* next);

    /**
       Find name in scalar context
     */
    virtual TypedNode::Ptr lookupScalar(const Ident::Name& name) const;
    /**
       Find name in functional context
     */
    virtual Func::Ptr lookupFunc(const Ident::Name& name) const;
    /**
       Insert name into scalar context. Name must be new one.
     */
    void insertScalar(const Ident::Name& name, TypedNode::Ptr typed);
    /**
       Insert name into functional context. Name must be new one.
     */
    void insertFunc(const Ident::Name& name, Func::Ptr func);

    virtual void to_json(json& j) const;

    static NameContext& globalContext();

  private:
    typedef std::map<Ident::Name, TypedNode::Ptr> ScalarContext;
    typedef std::map<Ident::Name, Func::Ptr> FuncContext;
    FuncContext funcContext;
    ScalarContext scalarContext;
    const NameContext* next; // next in chain of context (from local to global)
  };

  /**
   * Polymorphic name context
   *
   */
  class PolyNameContext : public NameContext {
  public:
    PolyNameContext(const NameContext* next = nullptr)
      : NameContext(next) {}
    TypedNode::Ptr lookupScalar(const Ident::Name& name) const override {
      TypedNode::Ptr node = NameContext::lookupScalar(name);
      return node ? node->clone() : node;
    }
    Func::Ptr lookupFunc(const Ident::Name& name) const override {
      Func::Ptr node = NameContext::lookupFunc(name);
      return node ? node->clone() : node;
    }
  };

  /**
   * Monomorphic name context
   */
  class MonoNameContext : public NameContext {
  public:
    MonoNameContext(const NameContext* next = nullptr)
      : NameContext(next) {}
  };
} // namespace compute

#endif // COMPUTE_NAMECONTEXT_HPP

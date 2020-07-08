#ifndef COMPUTE_NAMECONTEXT_HPP
#define COMPUTE_NAMECONTEXT_HPP

#include "Typed.hpp"

#include <map>

namespace compute {
  class NameContext {
  public:
    NameContext();

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
    typedef std::map<Ident::Name, Func::Ptr> FuncContext;
    FuncContext funcContext;
    ScalarContext scalarContext;
  };
} // namespace compute

#endif // COMPUTE_NAMECONTEXT_HPP

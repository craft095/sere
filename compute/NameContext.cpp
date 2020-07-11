#include "NameContext.hpp"
#include "Typed.hpp"

namespace compute {
  static void initializeContext(NameContext& context);

  NameContext::NameContext() {
    initializeContext(*this);
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


  FuncTypes math_binary {
               FuncType{{TypeId::UInt8, TypeId::UInt8}, TypeId::UInt8},
               FuncType{{TypeId::UInt16, TypeId::UInt16}, TypeId::UInt16},
               FuncType{{TypeId::UInt32, TypeId::UInt32}, TypeId::UInt32},
               FuncType{{TypeId::UInt64, TypeId::UInt64}, TypeId::UInt64},
               FuncType{{TypeId::SInt8, TypeId::SInt8}, TypeId::SInt8},
               FuncType{{TypeId::SInt16, TypeId::SInt16}, TypeId::SInt16},
               FuncType{{TypeId::SInt32, TypeId::SInt32}, TypeId::SInt32},
               FuncType{{TypeId::SInt64, TypeId::SInt64}, TypeId::SInt64},
               FuncType{{TypeId::Float, TypeId::Float}, TypeId::Float},
  };

  FuncTypes math_neg {
               FuncType{{TypeId::SInt8}, TypeId::SInt8},
               FuncType{{TypeId::SInt16}, TypeId::SInt16},
               FuncType{{TypeId::SInt32}, TypeId::SInt32},
               FuncType{{TypeId::SInt64}, TypeId::SInt64},
               FuncType{{TypeId::Float}, TypeId::Float},
  };

  FuncTypes cmp_binary {
               FuncType{{TypeId::UInt8, TypeId::UInt8}, TypeId::Bool},
               FuncType{{TypeId::UInt16, TypeId::UInt16}, TypeId::Bool},
               FuncType{{TypeId::UInt32, TypeId::UInt32}, TypeId::Bool},
               FuncType{{TypeId::UInt64, TypeId::UInt64}, TypeId::Bool},
               FuncType{{TypeId::SInt8, TypeId::SInt8}, TypeId::Bool},
               FuncType{{TypeId::SInt16, TypeId::SInt16}, TypeId::Bool},
               FuncType{{TypeId::SInt32, TypeId::SInt32}, TypeId::Bool},
               FuncType{{TypeId::SInt64, TypeId::SInt64}, TypeId::Bool},
               FuncType{{TypeId::Float, TypeId::Float}, TypeId::Bool},
               FuncType{{TypeId::Time, TypeId::Time}, TypeId::Bool},
  };

  FuncTypes eq_binary {
               FuncType{{TypeId::UInt8, TypeId::UInt8}, TypeId::Bool},
               FuncType{{TypeId::UInt16, TypeId::UInt16}, TypeId::Bool},
               FuncType{{TypeId::UInt32, TypeId::UInt32}, TypeId::Bool},
               FuncType{{TypeId::UInt64, TypeId::UInt64}, TypeId::Bool},
               FuncType{{TypeId::SInt8, TypeId::SInt8}, TypeId::Bool},
               FuncType{{TypeId::SInt16, TypeId::SInt16}, TypeId::Bool},
               FuncType{{TypeId::SInt32, TypeId::SInt32}, TypeId::Bool},
               FuncType{{TypeId::SInt64, TypeId::SInt64}, TypeId::Bool},
               FuncType{{TypeId::Float, TypeId::Float}, TypeId::Bool},
               FuncType{{TypeId::Time, TypeId::Time}, TypeId::Bool},
               FuncType{{TypeId::Bool, TypeId::Bool}, TypeId::Bool},
               FuncType{{TypeId::String, TypeId::String}, TypeId::Bool},
  };

  FuncTypes bool_binary {
               FuncType{{TypeId::Bool, TypeId::Bool}, TypeId::Bool},
  };

  FuncTypes bool_unary {
               FuncType{{TypeId::Bool}, TypeId::Bool},
  };

  FuncTypes sere_binary {
               FuncType{{TypeId::Sere, TypeId::Sere}, TypeId::Sere},
  };

  FuncTypes sere_unary {
               FuncType{{TypeId::Sere}, TypeId::Sere},
  };

  static void initializeContext(NameContext& context) {
    static Ident node {RE_LOC, "__builtin__"};
    FuncTypes add_sub {math_binary};
    add_sub.insert(FuncType{{TypeId::Time, TypeId::Time}, TypeId::Time} );

    FuncTypes mul{math_binary};
    mul.insert(FuncType{{TypeId::Time, TypeId::UInt64}, TypeId::Time} );
    mul.insert(FuncType{{TypeId::UInt64, TypeId::Time}, TypeId::Time} );

    context.insertFunc("__math_add", Func::create(&node, add_sub));
    context.insertFunc("__math_sub", Func::create(&node, add_sub));
    context.insertFunc("__math_mul", Func::create(&node, mul));
    context.insertFunc("__math_div", Func::create(&node, math_binary));

    context.insertFunc("__math_neg", Func::create(&node, math_neg));

    context.insertFunc("__bool_lt", Func::create(&node, cmp_binary));
    context.insertFunc("__bool_le", Func::create(&node, cmp_binary));
    context.insertFunc("__bool_gt", Func::create(&node, cmp_binary));
    context.insertFunc("__bool_ge", Func::create(&node, cmp_binary));

    context.insertFunc("__bool_eq", Func::create(&node, eq_binary));
    context.insertFunc("__bool_ne", Func::create(&node, eq_binary));

    context.insertFunc("__bool_and", Func::create(&node, bool_binary));
    context.insertFunc("__bool_or", Func::create(&node, bool_binary));

    context.insertFunc("__bool_not", Func::create(&node, bool_unary));

    context.insertFunc("__sere_intersect", Func::create(&node, sere_binary));
    context.insertFunc("__sere_union", Func::create(&node, sere_binary));
    context.insertFunc("__sere_concat", Func::create(&node, sere_binary));
    context.insertFunc("__sere_fusion", Func::create(&node, sere_binary));

    context.insertFunc("__sere_kleenestar", Func::create(&node, sere_unary));
    context.insertFunc("__sere_kleeneplus", Func::create(&node, sere_unary));
    context.insertFunc("__sere_complement", Func::create(&node, sere_unary));
  }
} // namespace compute

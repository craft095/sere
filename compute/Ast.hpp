#ifndef COMPUTE_AST_HPP
#define COMPUTE_AST_HPP

#include "ast/Common.hpp"
#include "ast/Located.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <memory>

#define TYPE_NAME(n)                           \
  template<>                                   \
  struct TypeName<n> {                         \
    static const char* name() { return #n; }   \
  };

using json = nlohmann::json;

namespace compute {
  template <typename T>
  void to_json(json& j, std::shared_ptr<T> a) {
    to_json(j, *a);
  }

  /**
     Helper trait to get name of a type.
     Used to pretty print AST nodes for debug.
  */
  template <typename T>
  struct TypeName {
    static const char* name();
  };

  template <typename T>
  class Literal;

  enum class SereLiteral
    {
     EPS,
    };

  typedef Literal<std::string> StringLit;
  typedef Literal<uint64_t> IntLit;
  typedef Literal<double> FloatLit;
  typedef Literal<bool> BoolLit;
  typedef Literal<SereLiteral> SereLit;

  TYPE_NAME(StringLit);
  TYPE_NAME(IntLit);
  TYPE_NAME(FloatLit);
  TYPE_NAME(BoolLit);
  TYPE_NAME(SereLit);

  class NameRef;
  class FuncCall;

  class ExpressionVisitor {
  public:
    virtual void visit(StringLit* x) = 0;
    virtual void visit(FloatLit* x) = 0;
    virtual void visit(IntLit* x) = 0;
    virtual void visit(BoolLit* x) = 0;
    virtual void visit(SereLit* x) = 0;
    virtual void visit(NameRef* x) = 0;
    virtual void visit(FuncCall* x) = 0;
  };

  class Node : public LocatedBase {
  public:
    Node(const Located& loc) : LocatedBase(loc) {}
    virtual void to_json(json& j) const = 0;
    const String pretty() const override;
  };

  extern void to_json(json& j, const Node& a);

  class Ident : public Node {
  public:
    typedef std::string Name;
    typedef std::shared_ptr<Ident> Ptr;
    Ident(const Located& loc, const Name& n) : Node(loc), name(n) {}
    const Name& getName() const { return name; }
    void to_json(json& j) const override;
  private:
    std::string name;
  };

  typedef std::vector<Ident::Ptr> Idents;

  enum class TypeId
    {
     None,
     Bool,
     Sere,
     UInt8,
     UInt16,
     UInt32,
     UInt64,
     SInt8,
     SInt16,
     SInt32,
     SInt64,
     Float,
     String,
     Time,
    };

  extern void to_json(json& j, TypeId t);

  class ArgDecl : public Node {
  public:
    typedef std::shared_ptr<ArgDecl> Ptr;
    ArgDecl(const Located& loc, TypeId tid, Ident::Ptr n)
      : Node(loc), typeId(tid), name(n) {}

    TypeId getTypeId() const { return typeId; }
    Ident::Ptr getName() const { return name; }
    void to_json(json& j) const override;
  private:
    TypeId typeId;
    Ident::Ptr name;
  };

  typedef std::vector<ArgDecl::Ptr> ArgDecls;

  class Expression : public Node {
  public:
    typedef std::shared_ptr<Expression> Ptr;
    Expression(const Located& loc) : Node(loc) {}
    virtual void accept(ExpressionVisitor& v) = 0;
  };

  template <typename T>
  class Literal : public Expression {
  public:
    typedef std::shared_ptr<Literal<T>> Ptr;
    Literal(const Located& loc, T val) : Expression(loc), value(val) {}
    T getValue() const { return value; }
    void accept(ExpressionVisitor& v) override { v.visit(this); }
    void to_json(json& j) const override {
      j = json {
        {"node",  "Literal" },
        {"value", value },
        {"type",  TypeName<Literal<T>>::name() },
      };
    }
  private:
    T value;
  };

  class NameRef : public Expression {
  public:
    typedef std::shared_ptr<NameRef> Ptr;
    NameRef(const Located& loc, Ident::Ptr i) : Expression(loc), name(i) {}
    Ident::Ptr getName() const { return name; }
    void accept(ExpressionVisitor& v) override { v.visit(this); }
    void to_json(json& j) const override;
  private:
    Ident::Ptr name;
  };

  typedef std::vector<Expression::Ptr> Expressions;

  class FuncCall : public Expression {
  public:
    typedef std::shared_ptr<FuncCall> Ptr;
    FuncCall(const Located& loc, Ident::Ptr i, const Expressions& as)
      : Expression(loc), name(i), args(as) {}
    Ident::Ptr getName() const { return name; }
    const Expressions& getArgs() const { return args; }
    void accept(ExpressionVisitor& v) override { v.visit(this); }
    void to_json(json& j) const override;
  private:
    Ident::Ptr name;
    Expressions args;
  };

  class LetDecl : public Node {
  public:
    typedef std::shared_ptr<LetDecl> Ptr;

    LetDecl(const Located& loc, Ident::Ptr n, const Idents& as, Expression::Ptr b)
      : Node(loc), name(n), args(as), body(b) {}

    Ident::Ptr getName() const { return name; }
    const Idents& getArgs() const { return args; }
    Expression::Ptr getBody() const { return body; }
    void to_json(json& j) const override;
  private:
    Ident::Ptr name;
    Idents args;
    Expression::Ptr body;
  };

  typedef std::vector<LetDecl::Ptr> LetDecls;

  class Root : public Node {
  public:
    typedef std::shared_ptr<Root> Ptr;
    Root(const Located& loc, const ArgDecls& args, const LetDecls& lets, Expression::Ptr expr)
      : Node(loc), argDecls(args), letDecls(lets), expression(expr)
    {}

    const ArgDecls& getArgDecls() const { return argDecls; }
    const LetDecls& getLetDecls() const { return letDecls; }
    Expression::Ptr getExpression() const { return expression; }

    void to_json(json& j) const override;
  private:
    ArgDecls argDecls;
    LetDecls letDecls;
    Expression::Ptr expression;
  };

} // namespace compute

#endif // COMPUTE_AST_HPP

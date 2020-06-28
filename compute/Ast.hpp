#ifndef COMPUTE_AST_HPP
#define COMPUTE_AST_HPP

#include "ast/Common.hpp"
#include "ast/Located.hpp"

#include <string>
#include <vector>
#include <memory>

namespace compute {
  class Node : public LocatedBase {
  public:
    Node(const Located& loc) : LocatedBase(loc) {}
    const String pretty() const override { return "not implemented"; }
    // virtual void accept(NodeVisitor& v) = 0;
  };

  class Ident : public LocatedBase {
  public:
    typedef std::string Name;
    typedef std::shared_ptr<Ident> Ptr;
    Ident(const Located& loc, const Name& n) : LocatedBase(loc), name(n) {}
    const Name& getName() const { return name; }
    const String pretty() const override { return "not implemented"; }
  private:
    std::string name;
  };

  typedef std::vector<Ident::Ptr> Idents;

  enum class TypeId
    {
     Bool,
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

  class ArgDecl : public Node {
  public:
    typedef std::shared_ptr<ArgDecl> Ptr;
    ArgDecl(const Located& loc, TypeId tid, Ident::Ptr n)
      : Node(loc), typeId(tid), name(n) {}

    TypeId getTypeId() const { return typeId; }
    Ident::Ptr getName() const { return name; }
  private:
    TypeId typeId;
    Ident::Ptr name;
  };

  typedef std::vector<ArgDecl::Ptr> ArgDecls;

  class Expression : public Node {
  public:
    typedef std::shared_ptr<Expression> Ptr;
    Expression(const Located& loc) : Node(loc) {}
  };

  template <typename T>
  class Literal : public Expression {
  public:
    typedef std::shared_ptr<Literal<T>> Ptr;
    Literal(const Located& loc, T val) : Expression(loc), value(val) {}
    T getValue() const { return value; }
  private:
    T value;
  };

  typedef Literal<std::string> StringLit;
  typedef Literal<uint64_t> IntLit;
  typedef Literal<double> FloatLit;

  class NameRef : public Expression {
  public:
    typedef std::shared_ptr<NameRef> Ptr;
    NameRef(const Located& loc, Ident::Ptr i) : Expression(loc), name(i) {}
    Ident::Ptr getName() const { return name; }
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
  private:
    Ident::Ptr name;
    Expressions args;
  };

  class UnaryOp : public Expression {
  public:
    typedef std::shared_ptr<UnaryOp> Ptr;
    enum class OpId
      {
       MATH_NEG,
       BOOL_NOT,
       SERE_COMPLEMENT,
       SERE_KLEENESTAR,
       SERE_KLEENEPLUS,
      };

    UnaryOp(const Located& loc, OpId op, Expression::Ptr a)
      : Expression(loc), opId(op), arg(a) {}
    OpId getOpId() const { return opId; }
    Expression::Ptr getArg() const { return arg; }
  private:
    OpId opId;
    Expression::Ptr arg;
  };

  class BinaryOp : public Expression {
  public:
    typedef std::shared_ptr<BinaryOp> Ptr;
    enum class OpId
      {
       MATH_ADD,
       MATH_SUB,
       MATH_DIV,
       MATH_MUL,
       BOOL_AND,
       BOOL_OR,
       BOOL_EQ,
       BOOL_NE,
       BOOL_LT,
       BOOL_LE,
       BOOL_GT,
       BOOL_GE,
       SERE_INTERSECT,
       SERE_UNION,
       SERE_CONCAT,
       SERE_FUSION,
      };

    BinaryOp(const Located& loc, OpId op, Expression::Ptr l, Expression::Ptr r)
      : Expression(loc), opId(op), lhs(l), rhs(r) {}
    OpId getOpId() const { return opId; }
    Expression::Ptr getLhs() const { return lhs; }
    Expression::Ptr getRhs() const { return rhs; }

  private:
    OpId opId;
    Expression::Ptr lhs;
    Expression::Ptr rhs;
  };

  class LetDecl : public Node {
  public:
    typedef std::shared_ptr<LetDecl> Ptr;

    LetDecl(const Located& loc, Ident::Ptr n, const Idents& as, Expression::Ptr b)
      : Node(loc), name(n), args(as), body(b) {}

    Ident::Ptr getName() const { return name; }
    const Idents& getArgs() const { return args; }
    Expression::Ptr getBody() const { return body; }
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

  private:
    ArgDecls argDecls;
    LetDecls letDecls;
    Expression::Ptr expression;
  };

} // namespace compute

#endif // COMPUTE_AST_HPP

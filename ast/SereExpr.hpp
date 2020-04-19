#ifndef AST_SEREEXPR_HPP
#define AST_SEREEXPR_HPP

#include <string>
#include <memory>
#include <boost/format.hpp>

#include "Located.hpp"
#include "BoolExpr.hpp"

class SereExpr;
class SereBool;
class SereEmpty;
class Union;
class Intersect;
class Concat;
class Fusion;
class KleeneStar;
class KleenePlus;
class Partial;

class SereVisitor {
public:
  virtual void visit(SereBool& v) = 0;
  virtual void visit(SereEmpty& v) = 0;
  virtual void visit(Union& v) = 0;
  virtual void visit(Intersect& v) = 0;
  virtual void visit(Concat& v) = 0;
  virtual void visit(Fusion& v) = 0;
  virtual void visit(KleeneStar& v) = 0;
  virtual void visit(KleenePlus& v) = 0;
  virtual void visit(Partial& v) = 0;
};

class SereExpr : public LocatedBase {
public:
  SereExpr(const Located& loc) : LocatedBase(loc) {}
  virtual void accept(SereVisitor& v) = 0;
};

class SereBool : public SereExpr {
public:
  SereBool(const Located& loc, Ptr<BoolExpr> expr_)
    : SereExpr(loc), expr(expr_) {}
  void accept(SereVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return expr->pretty();
  }
  Ptr<BoolExpr> getExpr() const { return expr; }
private:
  Ptr<BoolExpr> expr;
};

class SereEmpty : public SereExpr {
public:
  SereEmpty(const Located& loc_) : SereExpr(loc_) {}
  void accept(SereVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return "()";
  }
};

class Union : public SereExpr {
private:
  Ptr<SereExpr> lhs;
  Ptr<SereExpr> rhs;

public:
  Union(const Located& loc, Ptr<SereExpr> lhs_, Ptr<SereExpr> rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) | (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class Intersect : public SereExpr {
private:
  Ptr<SereExpr> lhs;
  Ptr<SereExpr> rhs;

public:
  Intersect(const Located& loc, Ptr<SereExpr> lhs_, Ptr<SereExpr> rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) & (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class Concat : public SereExpr {
private:
  Ptr<SereExpr> lhs;
  Ptr<SereExpr> rhs;

public:
  Concat(const Located& loc, Ptr<SereExpr> lhs_, Ptr<SereExpr> rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) ; (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class Fusion : public SereExpr {
private:
  Ptr<SereExpr> lhs;
  Ptr<SereExpr> rhs;

public:
  Fusion(const Located& loc, Ptr<SereExpr> lhs_, Ptr<SereExpr> rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) : (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class KleeneStar : public SereExpr {
private:
  Ptr<SereExpr> arg;

public:
  KleeneStar(const Located& loc, Ptr<SereExpr> arg_) : SereExpr(loc), arg(arg_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%)[*]") % arg->pretty()).str();
  }

  Ptr<SereExpr> getArg() const { return arg; }
};

class KleenePlus : public SereExpr {
private:
  Ptr<SereExpr> arg;

public:
  KleenePlus(const Located& loc, Ptr<SereExpr> arg_) : SereExpr(loc), arg(arg_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%)[+]") % arg->pretty()).str();
  }

  Ptr<SereExpr> getArg() const { return arg; }
};

class Partial : public SereExpr {
private:
  Ptr<SereExpr> arg;

public:
  Partial(const Located& loc, Ptr<SereExpr> arg_) : SereExpr(loc), arg(arg_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("PARTIAL(%1%)") % arg->pretty()).str();
  }

  Ptr<SereExpr> getArg() const { return arg; }
};

namespace nfasl {
  class Nfasl;
}

/**
 * Convert SERE into NFASL
 */
extern nfasl::Nfasl sereToNfasl(SereExpr& expr);

#define RE_SEREBOOL(expr) std::make_shared<SereBool>(RE_LOC, expr)
#define RE_EMPTY std::make_shared<SereEmpty>(RE_LOC)
#define RE_INTERSECT(u,v) std::make_shared<Intersect>(RE_LOC, u,v)
#define RE_UNION(u,v) std::make_shared<Union>(RE_LOC, u,v)
#define RE_CONCAT(u,v) std::make_shared<Concat>(RE_LOC, u,v)
#define RE_FUSION(u,v) std::make_shared<Fusion>(RE_LOC, u,v)
#define RE_STAR(u) std::make_shared<KleeneStar>(RE_LOC, u)
#define RE_PLUS(u) std::make_shared<KleenePlus>(RE_LOC, u)
#define RE_PARTIAL(u) std::make_shared<Partial>(RE_LOC, u)

#endif // AST_SEREEXPR_HPP

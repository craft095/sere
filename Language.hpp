#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include <set>
#include <vector>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <boost/format.hpp>

#include "Located.hpp"
#include "BoolExpr.hpp"

constexpr size_t maxAtoms = 64;

typedef std::string String;
struct VarName {
  static const char* names[26];

  size_t ix;

  const char* pretty() const {
    return names[ix];
  }

  friend bool operator== (VarName x, VarName y) { return x.ix == y.ix; }
  friend bool operator< (VarName x, VarName y) { return x.ix < y.ix; }
};

inline VarName make_varName(size_t ix) { return VarName {ix}; }

class SereExpr;

typedef std::shared_ptr<SereExpr> SereChildPtr;

class Variable;
class BoolValue;
class BoolNot;
class BoolAnd;
class BoolOr;

class SereBool;
class SereEmpty;
class Union;
class Intersect;
class Concat;
class Fusion;
class KleeneStar;
class KleenePlus;
class Partial;

template <typename T>
using Ptr = std::shared_ptr<T>;

class BoolVisitor {
public:
  virtual void visit(Variable& v) = 0;
  virtual void visit(BoolValue& v) = 0;
  virtual void visit(BoolNot& v) = 0;
  virtual void visit(BoolAnd& v) = 0;
  virtual void visit(BoolOr& v) = 0;
  virtual ~BoolVisitor() = default;
};

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

class LocatedBase {
public:
  void setLoc(const Located& loc_) {
    loc = loc_;
  }

  const Located& getLoc() const {
    return loc;
  }

  LocatedBase(const Located& loc_) : loc(loc_) {}
  virtual const String pretty() const = 0;
  virtual ~LocatedBase() {};
private:
  Located loc;
};

class BoolExpr : public LocatedBase {
public:
  BoolExpr(const Located& loc) : LocatedBase(loc) {}
  virtual void accept(BoolVisitor& v) = 0;
};

typedef std::shared_ptr<BoolExpr> BoolExprPtr;

class BoolNot : public BoolExpr {
public:
  BoolNot(BoolNot& nt) : BoolExpr(nt.getLoc()), arg(nt.getArg()) {}
  BoolNot(const Located& loc_, BoolExprPtr arg_) : BoolExpr(loc_), arg(arg_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("not (%1%)") % arg->pretty()).str();
  }

  BoolExprPtr getArg() const { return arg; }

private:
  BoolExprPtr arg;
};

class BoolAnd : public BoolExpr {
public:
  BoolAnd(BoolAnd& a) : BoolExpr(a.getLoc()), lhs(a.getLhs()), rhs(a.getRhs()) {}
  BoolAnd(const Located& loc_, BoolExprPtr lhs_, BoolExprPtr rhs_)
    : BoolExpr(loc_), lhs(lhs_), rhs(rhs_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("(%1%) && (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  BoolExprPtr getLhs() const { return lhs; }
  BoolExprPtr getRhs() const { return rhs; }

private:
  BoolExprPtr lhs;
  BoolExprPtr rhs;
};

class BoolOr : public BoolExpr {
public:
  BoolOr(BoolOr& a) : BoolExpr(a.getLoc()), lhs(a.getLhs()), rhs(a.getRhs()) {}
  BoolOr(const Located& loc_, BoolExprPtr lhs_, BoolExprPtr rhs_)
    : BoolExpr(loc_), lhs(lhs_), rhs(rhs_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("(%1%) || (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  BoolExprPtr getLhs() const { return lhs; }
  BoolExprPtr getRhs() const { return rhs; }

private:
  BoolExprPtr lhs;
  BoolExprPtr rhs;
};

class Variable : public BoolExpr {
private:
  VarName name;
public:
  Variable(Variable& v) : BoolExpr(v.getLoc()), name(v.getName()) {}
  Variable(const Located& loc, const VarName& n) : BoolExpr(loc), name (n) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    std::ostringstream st;
    st << "x" << name.ix;
    return st.str();
  }

  const VarName& getName() const { return name; }
  VarName& getNameRef() { return name; }
};

class BoolValue : public BoolExpr {
private:
  bool value;
public:
  BoolValue (BoolValue& v) : BoolExpr(v.getLoc()), value(v.getValue()) {}
  BoolValue (const Located& loc, bool v) : BoolExpr(loc), value (v) {}

  void accept(BoolVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return value ? "true" : "false";
  }

  bool getValue() const { return value; }
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
  SereChildPtr lhs;
  SereChildPtr rhs;

public:
  Union(const Located& loc, SereChildPtr lhs_, SereChildPtr rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) | (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class Intersect : public SereExpr {
private:
  SereChildPtr lhs;
  SereChildPtr rhs;

public:
  Intersect(const Located& loc, SereChildPtr lhs_, SereChildPtr rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

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
  Concat(const Located& loc, SereChildPtr lhs_, SereChildPtr rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

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
  Fusion(const Located& loc, SereChildPtr lhs_, SereChildPtr rhs_) : SereExpr(loc), lhs(lhs_), rhs(rhs_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%) : (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<SereExpr> getLhs() const { return lhs; }
  Ptr<SereExpr> getRhs() const { return rhs; }
};

class KleeneStar : public SereExpr {
private:
  SereChildPtr arg;

public:
  KleeneStar(const Located& loc, SereChildPtr arg_) : SereExpr(loc), arg(arg_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%)[*]") % arg->pretty()).str();
  }

  Ptr<SereExpr> getArg() const { return arg; }
};

class KleenePlus : public SereExpr {
private:
  SereChildPtr arg;

public:
  KleenePlus(const Located& loc, SereChildPtr arg_) : SereExpr(loc), arg(arg_) {}

  void accept(SereVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return (boost::format("(%1%)[+]") % arg->pretty()).str();
  }

  Ptr<SereExpr> getArg() const { return arg; }
};

class Partial : public SereExpr {
private:
  SereChildPtr arg;

public:
  Partial(const Located& loc, SereChildPtr arg_) : SereExpr(loc), arg(arg_) {}

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
 * Convert BoolExpr into equivalent compact representaion expr::Expr
 *
 * @param expr BoolExpr object
 * @return expr::Expr object
 */
extern expr::Expr boolExprToExpr(BoolExpr& expr);

/**
 * Get all variables in BoolExpr
 */
extern std::set<VarName> boolExprGetAtomics(BoolExpr& expr);

/**
 * Convert SERE into NFASL
 */
extern nfasl::Nfasl sereToNfasl(SereExpr& expr);

/**
 * Convert BoolExpr into run time representation
 */
extern void toRtPredicate(BoolExpr& expr,
                          std::vector<uint8_t>& data);

#define RE_LOC Located(Pos(__FILE__, __LINE__, 0))
#define RE_TRUE std::make_shared<BoolValue>(RE_LOC, true)
#define RE_FALSE std::make_shared<BoolValue>(RE_LOC, false)
#define RE_VAR(n) std::make_shared<Variable>(RE_LOC, make_varName(n))
#define RE_NOT(n) std::make_shared<BoolNot>(RE_LOC, n)
#define RE_AND(u,v) std::make_shared<BoolAnd>(RE_LOC, u ,v)
#define RE_OR(u,v) std::make_shared<BoolOr>(RE_LOC, u ,v)
#define RE_SEREBOOL(expr) std::make_shared<SereBool>(RE_LOC, expr)
#define RE_EMPTY std::make_shared<SereEmpty>(RE_LOC)
#define RE_INTERSECT(u,v) std::make_shared<Intersect>(RE_LOC, u,v)
#define RE_UNION(u,v) std::make_shared<Union>(RE_LOC, u,v)
#define RE_CONCAT(u,v) std::make_shared<Concat>(RE_LOC, u,v)
#define RE_FUSION(u,v) std::make_shared<Fusion>(RE_LOC, u,v)
#define RE_STAR(u) std::make_shared<KleeneStar>(RE_LOC, u)
#define RE_PLUS(u) std::make_shared<KleenePlus>(RE_LOC, u)
#define RE_PARTIAL(u) std::make_shared<Partial>(RE_LOC, u)

#endif // LANGUAGE_HPP

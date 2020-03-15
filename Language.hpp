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

typedef std::string String;
struct VarName {
  size_t ix;

  std::string pretty() const {
    return (boost::format("x%d") % int(ix)).str();
  }

  friend bool operator== (VarName x, VarName y) { return x.ix == y.ix; }
  friend bool operator< (VarName x, VarName y) { return x.ix < y.ix; }
};

class SereExpr;

typedef std::shared_ptr<SereExpr> SereChildPtr;

class Variable;
class BoolValue;
class BoolNot;
class BoolAnd;
class SereEmpty;
class Union;
class Intersect;
class Concat;
class KleeneStar;

template <typename T>
using Ptr = std::shared_ptr<T>;

class BoolVisitor {
public:
  virtual void visit(Variable& v) = 0;
  virtual void visit(BoolValue& v) = 0;
  virtual void visit(BoolNot& v) = 0;
  virtual void visit(BoolAnd& v) = 0;
  virtual ~BoolVisitor() = default;
};

class SereVisitor : public BoolVisitor {
public:
  virtual void visit(SereEmpty& v) = 0;
  virtual void visit(Union& v) = 0;
  virtual void visit(Intersect& v) = 0;
  virtual void visit(Concat& v) = 0;
  virtual void visit(KleeneStar& v) = 0;
};

class SereExpr {
public:
  void setLoc(const Located& loc_) {
    loc = loc_;
  }

  const Located& getLoc() const {
    return loc;
  }

  SereExpr(const Located& loc_) : loc(loc_) {}
  virtual void accept(SereVisitor& v) = 0;
  virtual const String pretty() const = 0;
  virtual ~SereExpr() {};
private:
  Located loc;
};

class SereEmpty : public SereExpr {
public:
  SereEmpty(const Located& loc_) : SereExpr(loc_) {}
  void accept(SereVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return "()";
  }
};

class BoolExpr : public SereExpr {
public:
  BoolExpr(const Located& loc_) : SereExpr(loc_) {}
  void accept(SereVisitor& v) override { accept(static_cast<BoolVisitor&>(v)); }
  virtual void accept(BoolVisitor& v) = 0;
};

typedef std::shared_ptr<BoolExpr> BoolExprPtr;

class BoolNot : public BoolExpr {
public:
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

class Literal : public BoolExpr {
public:
  Literal(const Located& loc_) : BoolExpr(loc_) {}
};

class Variable : public Literal {
private:
  VarName name;
public:
  Variable (const Located& loc, const VarName& n) : Literal(loc), name (n) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    std::ostringstream st;
    st << "x" << name.ix;
    return st.str();
  }

  const VarName& getName() const { return name; }
};


class BoolValue : public Literal {
private:
  bool value;
public:
  BoolValue (const Located& loc, bool v) : Literal(loc), value (v) {}

  void accept(BoolVisitor& v) override { v.visit(*this); }

  const String pretty() const override {
    return value ? "true" : "false";
  }

  bool getValue() const { return value; }
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

extern std::set<VarName> boolExprGetAtomics(BoolExpr& expr);

#endif // LANGUAGE_HPP

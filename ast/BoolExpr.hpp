#ifndef AST_BOOLEXPR_HPP
#define AST_BOOLEXPR_HPP

#include <set>
#include <vector>
#include <sstream>
#include <string>
#include <memory>
#include <boost/format.hpp>

#include "BoolExpr.hpp"
#include "ast/Located.hpp"
#include "ast/Common.hpp"

class Variable;
class BoolValue;
class BoolNot;
class BoolAnd;
class BoolOr;

class BoolVisitor {
public:
  virtual void visit(Variable& v) = 0;
  virtual void visit(BoolValue& v) = 0;
  virtual void visit(BoolNot& v) = 0;
  virtual void visit(BoolAnd& v) = 0;
  virtual void visit(BoolOr& v) = 0;
  virtual ~BoolVisitor() = default;
};

class BoolExpr : public LocatedBase {
public:
  BoolExpr(const Located& loc) : LocatedBase(loc) {}
  virtual void accept(BoolVisitor& v) = 0;
};

class BoolNot : public BoolExpr {
public:
  BoolNot(BoolNot& nt) : BoolExpr(nt.getLoc()), arg(nt.getArg()) {}
  BoolNot(const Located& loc_, Ptr<BoolExpr> arg_) : BoolExpr(loc_), arg(arg_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("not (%1%)") % arg->pretty()).str();
  }

  Ptr<BoolExpr> getArg() const { return arg; }

private:
  Ptr<BoolExpr> arg;
};

class BoolAnd : public BoolExpr {
public:
  BoolAnd(BoolAnd& a) : BoolExpr(a.getLoc()), lhs(a.getLhs()), rhs(a.getRhs()) {}
  BoolAnd(const Located& loc_, Ptr<BoolExpr> lhs_, Ptr<BoolExpr> rhs_)
    : BoolExpr(loc_), lhs(lhs_), rhs(rhs_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("(%1%) && (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<BoolExpr> getLhs() const { return lhs; }
  Ptr<BoolExpr> getRhs() const { return rhs; }

private:
  Ptr<BoolExpr> lhs;
  Ptr<BoolExpr> rhs;
};

class BoolOr : public BoolExpr {
public:
  BoolOr(BoolOr& a) : BoolExpr(a.getLoc()), lhs(a.getLhs()), rhs(a.getRhs()) {}
  BoolOr(const Located& loc_, Ptr<BoolExpr> lhs_, Ptr<BoolExpr> rhs_)
    : BoolExpr(loc_), lhs(lhs_), rhs(rhs_) {}
  void accept(BoolVisitor& v) override { v.visit(*this); }
  const String pretty() const override {
    return (boost::format("(%1%) || (%2%)") % lhs->pretty() % rhs->pretty()).str();
  }

  Ptr<BoolExpr> getLhs() const { return lhs; }
  Ptr<BoolExpr> getRhs() const { return rhs; }

private:
  Ptr<BoolExpr> lhs;
  Ptr<BoolExpr> rhs;
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
 * Convert BoolExpr into run time representation
 */
extern void toRtPredicate(BoolExpr& expr,
                          std::vector<uint8_t>& data);

#define RE_TRUE std::make_shared<BoolValue>(RE_LOC, true)
#define RE_FALSE std::make_shared<BoolValue>(RE_LOC, false)
#define RE_VAR(n) std::make_shared<Variable>(RE_LOC, make_varName(n))
#define RE_NOT(n) std::make_shared<BoolNot>(RE_LOC, n)
#define RE_AND(u,v) std::make_shared<BoolAnd>(RE_LOC, u ,v)
#define RE_OR(u,v) std::make_shared<BoolOr>(RE_LOC, u ,v)

#endif // AST_BOOLEXPR_HPP

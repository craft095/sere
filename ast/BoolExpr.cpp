#include "BoolExpr.hpp"
#include "boolean/Expr.hpp"

class ToExpr : public BoolVisitor {
private:
  boolean::Expr result;
public:
  ToExpr(BoolExpr& expr) {
    expr.accept(*this);
  }

  boolean::Expr getResult() const { return result; }

  void visit(Variable& v) override {
    result = boolean::Expr::var(v.getName().ix);
  }

  void visit(BoolValue& e) override {
    result = boolean::Expr::value(e.getValue());
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    boolean::Expr arg = result;
    result = !arg;
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    boolean::Expr lhs = result;
    v.getRhs()->accept(*this);
    boolean::Expr rhs = result;
    result = lhs && rhs;
  }

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    boolean::Expr lhs = result;
    v.getRhs()->accept(*this);
    boolean::Expr rhs = result;
    result = lhs || rhs;
  }
};

boolean::Expr boolExprToExpr(BoolExpr& expr) {
  return ToExpr(expr).getResult();
}

class GetAtomics : public BoolVisitor {
private:
  std::set<VarName> vars;
public:
  GetAtomics(BoolExpr& expr) {
    expr.accept(*this);
  }

  const std::set<VarName>&  getResult() const { return vars; }

  void visit(Variable& v) override {
    vars.insert(v.getName());
  }

  void visit(BoolValue&) override {
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
};

std::set<VarName> boolExprGetAtomics(BoolExpr& expr) {
  GetAtomics ga{expr};
  return ga.getResult();
}

void toRtPredicate(BoolExpr& expr,
                   std::vector<uint8_t>& data) {
  toRtPredicate(boolExprToExpr(expr), data);
}

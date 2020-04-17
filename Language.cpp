#include "Language.hpp"
#include "Nfasl.hpp"

using Nfasl = nfasl::Nfasl;

const char* VarName::names[] = {
  "x0",
  "x1",
  "x2",
  "x3",
  "x4",
  "x5",
  "x6",
  "x7",
  "x8",
  "x9",
  "x10",
  "x11",
  "x12",
  "x13",
  "x14",
  "x15",
  "x16",
  "x17",
  "x18",
  "x19",
  "x20",
  "x21",
  "x22",
  "x23",
  "x24",
  "x25",
};

class ToExpr : public BoolVisitor {
private:
  expr::Expr result;
public:
  ToExpr(BoolExpr& expr) {
    expr.accept(*this);
  }

  expr::Expr getResult() const { return result; }

  void visit(Variable& v) override {
    result = expr::Expr::var(v.getName().ix);
  }

  void visit(BoolValue& e) override {
    result = expr::Expr::value(e.getValue());
  }

  void visit(BoolNot& v) override {
    v.getArg()->accept(*this);
    expr::Expr arg = result;
    result = !arg;
  }

  void visit(BoolAnd& v) override {
    v.getLhs()->accept(*this);
    expr::Expr lhs = result;
    v.getRhs()->accept(*this);
    expr::Expr rhs = result;
    result = lhs && rhs;
  }

  void visit(BoolOr& v) override {
    v.getLhs()->accept(*this);
    expr::Expr lhs = result;
    v.getRhs()->accept(*this);
    expr::Expr rhs = result;
    result = lhs || rhs;
  }
};

expr::Expr boolExprToExpr(BoolExpr& expr) {
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


class SereToNfasl : public SereVisitor {
private:
  Nfasl result;
public:
  SereToNfasl(SereExpr& expr) {
    expr.accept(*this);
  }

  Nfasl getResult() const {
    return result;
  }

  void visit(Variable& v) override {
    result = nfasl::phi(boolExprToExpr(v));
  }

  void visit(BoolValue& v) override {
    result = nfasl::phi(boolExprToExpr(v));
  }

  void visit(BoolNot& v) override {
    result = nfasl::phi(boolExprToExpr(v));
  }

  void visit(BoolAnd& v) override {
    result = nfasl::phi(boolExprToExpr(v));
  }

  void visit(BoolOr& v) override {
    result = nfasl::phi(boolExprToExpr(v));
  }

  void visit(SereEmpty& ) override {
    result = nfasl::eps();
  }

  void visit(Union& v) override {
    Nfasl lhs = sereToNfasl(*v.getLhs());
    Nfasl rhs = sereToNfasl(*v.getRhs());

    result = nfasl::unions(lhs, rhs);
  }

  void visit(Intersect& v) override {
    Nfasl lhs = sereToNfasl(*v.getLhs());
    Nfasl rhs = sereToNfasl(*v.getRhs());

    result = nfasl::intersects(lhs, rhs);
  }

  void visit(Concat& v) override {
    Nfasl lhs = sereToNfasl(*v.getLhs());
    Nfasl rhs = sereToNfasl(*v.getRhs());

    result = nfasl::concat(lhs, rhs);
  }

  void visit(Fusion& v) override {
    Nfasl lhs = sereToNfasl(*v.getLhs());
    Nfasl rhs = sereToNfasl(*v.getRhs());

    result = nfasl::fuse(lhs, rhs);
  }

  void visit(KleeneStar& v) override {
    Nfasl arg = sereToNfasl(*v.getArg());

    result = nfasl::kleeneStar(arg);
  }

  void visit(KleenePlus& v) override {
    Nfasl arg = sereToNfasl(*v.getArg());

    result = nfasl::kleenePlus(arg);
  }

  void visit(Partial& v) override {
    Nfasl arg = sereToNfasl(*v.getArg());

    result = nfasl::partial(arg);
  }
};

Nfasl sereToNfasl(SereExpr& expr) {
  return SereToNfasl(expr).getResult();
}

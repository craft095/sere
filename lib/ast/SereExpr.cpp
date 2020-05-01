#include "ast/SereExpr.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"

using Nfasl = nfasl::Nfasl;

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

  void visit(SereBool& v) override {
    result = nfasl::phi(boolExprToExpr(*v.getExpr()));
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

  void visit(Complement& v) override {
    Nfasl arg = sereToNfasl(*v.getArg());

    nfasl::complement(arg, result);
  }
};

Nfasl sereToNfasl(SereExpr& expr) {
  return SereToNfasl(expr).getResult();
}

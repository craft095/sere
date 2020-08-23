#include "CompareExprs.hpp"

class RemapVars : public SereVisitor, public BoolVisitor {
  std::map<size_t, size_t> remap;
public:
  RemapVars (parser::ParseResult& self,
             std::map<std::string, size_t> theirVars) {
    REQUIRE(self.vars.size() == theirVars.size());

    for (auto& i0 : self.vars) {
      REQUIRE(theirVars.find(i0.first) != theirVars.end());
      remap[i0.second] = theirVars[i0.first];
    }

    self.expr->accept(*this);
  }

  void visit(Variable& v) override {
    size_t ix = v.getName().ix;
    assert(remap.find(ix) != remap.end());
    v.getNameRef().ix = remap[ix];
  }
  void visit(BoolValue& ) override {
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
  void visit(SereBool& v) override {
    v.getExpr()->accept(*this);
  }
  void visit(SereEmpty& ) override {
  }
  void visit(Union& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
  void visit(Intersect& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
  void visit(Concat& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
  void visit(Fusion& v) override {
    v.getLhs()->accept(*this);
    v.getRhs()->accept(*this);
  }
  void visit(KleeneStar& v) override {
    v.getArg()->accept(*this);
  }
  void visit(KleenePlus& v) override {
    v.getArg()->accept(*this);
  }
  void visit(Partial& v) override {
    v.getArg()->accept(*this);
  }
  void visit(Complement& v) override {
    v.getArg()->accept(*this);
  }
};

void prepareExpr(Ptr<SereExpr> expr, rt::Nfasl& rtNfasl) {
  nfasl::Nfasl expr0 = sereToNfasl(*expr);
  nfasl::Nfasl expr1;
  nfasl::clean(expr0, expr1);
  toRt(expr1, rtNfasl);
}

void prepareExprs(parser::ParseResult r0,
                  parser::ParseResult r1,
                  rt::Nfasl& rtNfasl0,
                  rt::Nfasl& rtNfasl1) {
  REQUIRE(r0.vars.size() == r1.vars.size());

  RemapVars remapper{r1, r0.vars};

  prepareExpr(r0.expr, rtNfasl0);
  prepareExpr(r1.expr, rtNfasl1);
}

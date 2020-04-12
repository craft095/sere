#include "Parser.hpp"
#include "Match.hpp"
#include "Nfasl.hpp"
#include "BisimNfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Tools.hpp"
#include "Letter.hpp"
#include "GenLetter.hpp"
#include "EvalRtNfasl.hpp"

#include <sstream>
#include "catch2/catch.hpp"

class RemapVars : public SereVisitor {
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
};

template <typename T>
void checkAlt(Ptr<SereExpr> u, Ptr<SereExpr> v) {
  if (dynamic_cast<T*>(u.get()) != nullptr) {
    CHECK(dynamic_cast<T*>(v.get()) != nullptr);
  }
}

void checkEquiv(Ptr<SereExpr> u, Ptr<SereExpr> v) {
  checkAlt<SereEmpty>(u,v);
  checkAlt<BoolNot>(u,v);
  checkAlt<BoolValue>(u,v);
  checkAlt<Variable>(u,v);
  checkAlt<Union>(u,v);
  checkAlt<Intersect>(u,v);
  checkAlt<Concat>(u,v);
  checkAlt<KleeneStar>(u,v);
  checkAlt<KleenePlus>(u,v);
}

void checkExpr(Ptr<SereExpr> expr0, const char* text) {
  std::istringstream stream(text);
  Ptr<SereExpr> expr1 = parser::parse(stream).expr;
  checkEquiv(expr0, expr1);
}

void prepareExpr(Ptr<SereExpr> expr, rt::Nfasl& rtNfasl) {
  nfasl::Nfasl expr0 = sereToNfasl(*expr);
  nfasl::Nfasl expr1;
  nfasl::clean(expr0, expr1);
  toRt(expr1, rtNfasl);
}

void prepareExprs(const char* u, const char* v, rt::Nfasl& rtNfasl0, rt::Nfasl& rtNfasl1) {
  std::istringstream stream0(u);
  parser::ParseResult r0 = parser::parse(stream0);
  std::istringstream stream1(v);
  parser::ParseResult r1 = parser::parse(stream1);

  REQUIRE(r0.vars.size() == r1.vars.size());

  RemapVars remapper{r1, r0.vars};

  prepareExpr(r0.expr, rtNfasl0);
  prepareExpr(r1.expr, rtNfasl1);
}

TEST_CASE("Parser") {
  SECTION("empty") {
    checkExpr(RE_EMPTY, "()");
  }
  SECTION("boolean") {
    checkExpr(RE_TRUE,
              "true");
    checkExpr(RE_FALSE,
              "false");
    checkExpr(RE_VAR(0),
              "var1");
    checkExpr(RE_NOT(RE_TRUE),
              "!true");
    checkExpr(RE_AND(RE_VAR(0), RE_NOT(RE_VAR(1))),
              "var0 && !var1");
  }
  SECTION("sere") {
    checkExpr(RE_CONCAT
              (RE_VAR(0),
               RE_VAR(1)),
              "var0 ; var1");
    checkExpr(RE_UNION
              (RE_VAR(0),
               RE_VAR(1)),
              "var0 | var1");
    checkExpr(RE_INTERSECT
              (RE_VAR(0),
               RE_VAR(1)),
              "var0 & var1");
    checkExpr(RE_UNION
              (RE_VAR(0),
               RE_VAR(1)),
              "var0 | var1");
    checkExpr(RE_UNION
              (RE_VAR(0),
               (RE_INTERSECT
                (RE_VAR(0),
                 RE_VAR(1)))),
              "var0 | var0 & var1");
    checkExpr(RE_STAR(RE_TRUE),
              "true [*]");
  }
}

#define COMPARE_EXPRS(u, v)                                           \
  {                                                                   \
  rt::Nfasl rtNfasl0, rtNfasl1;                                       \
  prepareExprs(u, v, rtNfasl0, rtNfasl1);                             \
                                                                      \
  auto atoms = rtNfasl0.atomicCount;                                  \
  for (size_t cnt = 0; cnt < 100; ++cnt) {                            \
    auto word0 = WordGenerator::make(atoms, 0, 8);                    \
    Match res0 = evalRtNfasl(rtNfasl0, word0);                        \
    Match res1 = evalRtNfasl(rtNfasl1, word0);                        \
                                                                      \
    CHECK(res0 == res1);                                              \
  }                                                                   \
  }

TEST_CASE("Parser Transforms: PERMUTE") {
  COMPARE_EXPRS("PERMUTE(a)", "a");
  COMPARE_EXPRS("PERMUTE(a,b)", "(a;b) | (b;a)");
  COMPARE_EXPRS("PERMUTE(a,b,c)",
                "(a;b;c) | (a;c;b) | (b;a;c) |"
                "(b;c;a) | (c;a;b) | (c;b;a)" );
}

TEST_CASE("Parser Transforms: RANGES") {
  COMPARE_EXPRS("a{0}", "()");
  COMPARE_EXPRS("a{1}", "a");
  COMPARE_EXPRS("a{3}", "a;a;a");
  COMPARE_EXPRS("(a|b){0,}", "(a|b)[*]");
  COMPARE_EXPRS("(a|b){3,}", "(a|b);(a|b);(a|b)[+]");
  COMPARE_EXPRS("a{0,0}", "()");
  COMPARE_EXPRS("a{0,1}", "() | a");
  COMPARE_EXPRS("a{1,1}", "a");
  COMPARE_EXPRS("a{0,2}", "() | a | (a;a)");
  COMPARE_EXPRS("a{2,5}", "(a;a) | (a;a;a) | (a;a;a;a) | (a;a;a;a;a)");
}

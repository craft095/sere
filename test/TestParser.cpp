#include "Match.hpp"
#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "rt/RtNfasl.hpp"

#include "test/Tools.hpp"
#include "test/Letter.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalRt.hpp"

#include "test/CompareExprs.hpp"

#include <sstream>
#include "catch2/catch.hpp"

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
  checkAlt<Complement>(u,v);
}

void checkExpr(Ptr<SereExpr> expr0, const char* text) {
  std::istringstream stream(text);
  Ptr<SereExpr> expr1 = parser::parse(stream).expr;
  checkEquiv(expr0, expr1);
}

void checkBoolExpr(Ptr<BoolExpr> expr, const char* text) {
  checkExpr(RE_SEREBOOL(expr), text);
}

#define COMPARE_EXPRS(u, v)                                             \
  {                                                                     \
    std::istringstream stream0(u);                                      \
    parser::ParseResult r0 = parser::parse(stream0);                    \
    std::istringstream stream1(v);                                      \
    parser::ParseResult r1 = parser::parse(stream1);                    \
    COMPARE_EXPRS0(r0, r1);                                             \
  }

TEST_CASE("Parser") {
  SECTION("empty") {
    checkExpr(RE_EMPTY, "()");
  }
  SECTION("boolean") {
    checkBoolExpr(RE_TRUE,
                  "true");
    checkBoolExpr(RE_FALSE,
                  "false");
    checkBoolExpr(RE_VAR(0),
                  "var1");
    checkBoolExpr(RE_NOT(RE_TRUE),
                  "!true");
    checkBoolExpr(RE_AND(RE_VAR(0), RE_NOT(RE_VAR(1))),
                  "var0 && !var1");
  }
  SECTION("sere") {
    checkExpr(RE_CONCAT
              (RE_SEREBOOL(RE_VAR(0)),
               RE_SEREBOOL(RE_VAR(1))),
              "var0 ; var1");
    checkExpr(RE_UNION
              (RE_SEREBOOL(RE_VAR(0)),
               RE_SEREBOOL(RE_VAR(1))),
              "var0 | var1");
    checkExpr(RE_INTERSECT
              (RE_SEREBOOL(RE_VAR(0)),
               RE_SEREBOOL(RE_VAR(1))),
              "var0 & var1");
    checkExpr(RE_UNION
              (RE_SEREBOOL(RE_VAR(0)),
               RE_SEREBOOL(RE_VAR(1))),
              "var0 | var1");
    checkExpr(RE_UNION
              (RE_SEREBOOL(RE_VAR(0)),
               (RE_INTERSECT
                (RE_SEREBOOL(RE_VAR(0)),
                 RE_SEREBOOL(RE_VAR(1))))),
              "var0 | var0 & var1");
    checkExpr(RE_STAR(RE_SEREBOOL(RE_TRUE)),
              "true [*]");
  }
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

TEST_CASE("Parser Transforms: ABORT") {
  COMPARE_EXPRS("ABORT(!a|b,c)", "(PARTIAL(!a|b) ; c) | (!a|b)");
}

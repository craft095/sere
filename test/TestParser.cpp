#include "Parser.hpp"

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
  //checkAlt<KleenePlus>(u,v);
}

void checkExpr(Ptr<SereExpr> expr0, const char* text) {
  std::istringstream stream(text);
  Ptr<SereExpr> expr1 = parser::parse(stream);
  checkEquiv(expr0, expr1);
}

TEST_CASE("Parsre") {
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

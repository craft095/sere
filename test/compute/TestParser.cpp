#include "compute/Typed.hpp"
#include "compute/NameContext.hpp"
#include "compute/Infer.hpp"
#include "compute/Parser.hpp"

#include "test/Tools.hpp"
#include "test/Letter.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalRt.hpp"

#include "Match.hpp"
#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "rt/RtNfasl.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include "catch2/catch.hpp"

using namespace compute;

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

Root::Ptr parseString(const char* text) {
  std::istringstream stream(text);
  return parse("<compute test>", stream);
}

void prepareExpr(Ptr<SereExpr> expr, rt::Nfasl& rtNfasl) {
  nfasl::Nfasl expr0 = sereToNfasl(*expr);
  nfasl::Nfasl expr1;
  nfasl::clean(expr0, expr1);
  toRt(expr1, rtNfasl);
}

void prepareExprs(const char* u, const char* v, rt::Nfasl& rtNfasl0, rt::Nfasl& rtNfasl1) {
  Root::Ptr root = parseString(u);
  TypedNode::Ptr typed = infer(NameContext::globalContext(), root);
  parser::ParseResult r0;
  toSere(root, typed, r0);

  std::istringstream stream1(v);
  parser::ParseResult r1 = parser::parse(stream1);

  REQUIRE(r0.vars.size() == r1.vars.size());

  RemapVars remapper{r1, r0.vars};

  prepareExpr(r0.expr, rtNfasl0);
  prepareExpr(r1.expr, rtNfasl1);
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

TEST_CASE("Compute/Sere equivalence") {
  COMPARE_EXPRS("()", "()");
  COMPARE_EXPRS("true", "true");
  COMPARE_EXPRS("false", "false");
  COMPARE_EXPRS("true || false", "true || false");
  COMPARE_EXPRS("true && false", "true && false");
  COMPARE_EXPRS("a :: Bool ; a", "a");
  COMPARE_EXPRS("a :: Bool ; a & a", "a & a");
}

TEST_CASE("Parser") {
  #if 0
  Root::Ptr ast0 = parseString("12");
  std::cout << ast0->pretty() << std::endl;

  Root::Ptr ast1 = parseString("a");
  std::cout << ast1->pretty() << std::endl;

  Root::Ptr ast2 = parseString("()");
  std::cout << ast2->pretty() << std::endl;

  Root::Ptr ast3 = parseString("12+(-4200)");
  std::cout << ast3->pretty() << std::endl;

  Root::Ptr ast4 = parseString("12+a");
  std::cout << ast4->pretty() << std::endl;
#endif

  NameContext& context { NameContext::globalContext() };

  // Root::Ptr ast3 = parseString("12+(-4200)");
  // std::cout << ast3->pretty() << std::endl;
  // TypedNode::Ptr typed = inferExpr(context, ast3->getExpression());
  // std::cout << typed->pretty() << std::endl;

  SECTION("Constrain/Scalar") {
    Root::Ptr ast = parseString("25");
    TypedNode::Ptr typed = infer(context, ast);
    typed->constrain({TypeId::SInt16});
    CHECK(typed->getTypeIds() == TypeIds { TypeId::SInt16 });
  }
  SECTION("Constrain/Scalar/ScalarTypeMismatch") {
    Root::Ptr ast = parseString("-25");
    TypedNode::Ptr typed = infer(context, ast);
    CHECK_THROWS_AS(typed->constrain({TypeId::UInt16}), ScalarTypeMismatch);
  }
  SECTION("Constrain/Apply") {
    Root::Ptr ast = parseString("25-12");
    TypedNode::Ptr typed = infer(context, ast);
    typed->constrain({TypeId::SInt16});
    CHECK(typed->getTypeIds() == TypeIds { TypeId::SInt16 });
  }
  SECTION("Constrain/Apply/BadApplication") {
    Root::Ptr ast = parseString("true - 12");
    CHECK_THROWS_AS(infer(context, ast), BadApplication);
  }
  SECTION("NameNotFound") {
    Root::Ptr ast = parseString("catch22");
    CHECK_THROWS_AS(infer(context, ast), NameNotFound);
  }
  SECTION("FuncExpected") {
    Root::Ptr ast = parseString("a :: Bool ; a(12)");
    CHECK_THROWS_AS(infer(context, ast), FuncExpected);
  }
  // SECTION("ScalarExpected") {
  //   Root::Ptr ast = parseString("permute + 12");
  //   CHECK_THROWS_AS(infer(context, ast), ScalarExpected);
  // }
  SECTION("ArgDecls/UInt8") {
    Root::Ptr ast = parseString("ab \n:: \nUInt8 \n; \nab");
    TypedNode::Ptr typed = infer(context, ast);
    CHECK(typed->getTypeIds() ==
                    TypeIds {
                     TypeId::UInt8, TypeId::UInt16, TypeId::UInt32, TypeId::UInt64,
                     TypeId::SInt16, TypeId::SInt32, TypeId::SInt64, TypeId::Float
                    });
  }
  SECTION("ArgDecls/Bool") {
    Root::Ptr ast = parseString("a :: Bool ; a");
    TypedNode::Ptr typed = infer(context, ast);
    CHECK(typed->getTypeIds() ==
                    TypeIds {
                     TypeId::Bool, TypeId::Sere,
                    });
  }
  SECTION("LetDecls/Scalar/Bool") {
    Root::Ptr ast = parseString("a :: Bool ; let b = a || a ; b");
    TypedNode::Ptr typed = infer(context, ast);
    CHECK(typed->getTypeIds() ==
                    TypeIds {
                     TypeId::Bool, TypeId::Sere,
                    });
  }
}

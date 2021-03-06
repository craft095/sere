#include "compute/Typed.hpp"
#include "compute/NameContext.hpp"
#include "compute/Infer.hpp"
#include "compute/Parser.hpp"

#include "test/CompareExprs.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include "catch2/catch.hpp"

using namespace compute;

Root::Ptr parseString(const char* text) {
  std::istringstream stream(text);
  return parse("<compute test>", stream);
}

#define COMPARE_EXPRS(u, v)                                           \
  {                                                                   \
    std::istringstream stream0(u);                                    \
    Root::Ptr root = parse("<compute test>", stream0);                \
    TypedNode::Ptr typed = infer(NameContext::globalContext(), root); \
    parser::ParseResult r0;                                           \
    toSere(root, typed, r0);                                          \
                                                                      \
    std::istringstream stream1(v);                                    \
    parser::ParseResult r1 = parser::parse(stream1);                  \
    COMPARE_EXPRS0(r0, r1)                                            \
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

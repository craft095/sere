#include "compute/Typed.hpp"
#include "compute/NameContext.hpp"
#include "compute/Infer.hpp"
#include "compute/Parser.hpp"

#include <sstream>
#include <iostream>
#include <algorithm>
#include "catch2/catch.hpp"

using namespace compute;

Root::Ptr parseString(const char* text) {
  std::istringstream stream(text);
  return parse("<compute test>", stream);
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

  NameContext context;

  Root::Ptr ast3 = parseString("12+(-4200)");
  std::cout << ast3->pretty() << std::endl;
  TypedNode::Ptr typed = inferExpr(context, ast3->getExpression());
  std::cout << typed->pretty() << std::endl;

  SECTION("Constrain/Scalar") {
    Root::Ptr ast = parseString("25");
    TypedNode::Ptr typed = infer(context, ast);
    typed->constrain({TypeId::SInt16});
    CHECK(typed->getTypeIds() == TypeIds { TypeId::SInt16 });
  }
  SECTION("Constrain/Apply") {
    Root::Ptr ast = parseString("25-12");
    TypedNode::Ptr typed = infer(context, ast);
    typed->constrain({TypeId::SInt16});
    CHECK(typed->getTypeIds() == TypeIds { TypeId::SInt16 });
  }
  SECTION("ArgDecls/UInt8") {
    Root::Ptr ast = parseString("a :: UInt8 ; a");
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
}

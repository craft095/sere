#include "compute/Typed.hpp"
#include "compute/Parser.hpp"

#include <sstream>
#include <iostream>
#include "catch2/catch.hpp"

using namespace compute;

Root::Ptr parseString(const char* text) {
  std::istringstream stream(text);
  return parse("<compute test>", stream);
}

TEST_CASE("Parser") {
  Root::Ptr ast0 = parseString("12");
  std::cout << ast0->pretty() << std::endl;

  Root::Ptr ast1 = parseString("a");
  std::cout << ast1->pretty() << std::endl;

  Root::Ptr ast2 = parseString("()");
  std::cout << ast2->pretty() << std::endl;

  Root::Ptr ast3 = parseString("12+42");
  std::cout << ast3->pretty() << std::endl;

  Root::Ptr ast4 = parseString("12+a");
  std::cout << ast4->pretty() << std::endl;

  NameContext context;
  FuncTypes add {FuncType{{TypeId::UInt32, TypeId::UInt32}, TypeId::UInt32}};
  context.insertFunc("__math_add", Func::create(add));

  TypedNode::Ptr typed = inferTypes(context, ast3->getExpression());
  std::cout << typed->pretty() << std::endl;
}

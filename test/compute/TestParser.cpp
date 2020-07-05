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

  Root::Ptr ast3 = parseString("12+a");
  std::cout << ast3->pretty() << std::endl;
}

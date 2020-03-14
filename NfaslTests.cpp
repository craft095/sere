#include <iostream>
#include <memory>

#define CATCH_CONFIG_RUNNER
#include "Nfasl.hpp"

TEST_CASE("Nfasl") {
}

int main(int argc, char **argv) {
  int result = Catch::Session().run( argc, argv );
  return result;
}

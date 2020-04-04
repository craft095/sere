#ifdef GTEST
#include "gtest/gtest.h"
#endif

#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

int main(int argc, char **argv) {
#ifdef GTEST
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  if (result != 0) {
    return result;
  }
#endif
  int result = Catch::Session().run( argc, argv );

  return result;
}

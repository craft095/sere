#include <iostream>
#include <memory>

// #include "gtest/gtest.h"

#define CATCH_CONFIG_RUNNER

#include "Language.hpp"

#include "Nfasl.hpp"
#include "Letter.hpp"
#include "EvalSere.hpp"
#include "TestLetter.hpp"
#include "Algo.hpp"
#include "Z3.hpp"

#include "catch2/catch.hpp"

#include "TestTools.hpp"
#include "TestBoolExpr.hpp"
#include "TestZ3.hpp"
#include "NfaslTests.hpp"

TEST_CASE("Sere") {
  SECTION("empty") {
    CHECK(evalSere(*RE_EMPTY, {}) == Match_Ok);
    CHECK(evalSere(*RE_EMPTY, {{"",""}}) == Match_Failed);
  }
  SECTION("boolean") {
    CHECK(evalSere(*RE_TRUE, {{"y", "x"}}) == Match_Ok);
    CHECK(evalSere(*RE_FALSE, {{"y", "x"}}) == Match_Failed);
    CHECK(evalSere(*RE_NOT(RE_TRUE), {{"y", "x"}}) == Match_Failed);
    CHECK(evalSere(*RE_VAR('y'), {{"y", "x"}}) == Match_Ok);
    CHECK(evalSere(*RE_VAR('x'), {{"y", "x"}}) == Match_Failed);

    SereChildPtr tc = RE_AND(RE_TRUE, RE_NOT(RE_VAR('x')));
    CHECK(evalSere(*tc, {{"y", "x"}}) == Match_Ok);
    CHECK(evalSere(*tc, {{"x", "y"}}) == Match_Failed);
    CHECK(evalSere(*tc, {{"y", "x"}, {"y", "x"}}) == Match_Failed);
    CHECK(evalSere(*tc, {}) == Match_Partial);
    // ASSERT_EQ(evalSere(*tc, {{"z", "y"}}), Match_Failed);
  }

  SECTION("concat") {
    CHECK(evalSere(*RE_CONCAT
                   (RE_VAR('x'),
                    RE_VAR('y')),
                   {}) == Match_Partial);
    CHECK(evalSere(*RE_CONCAT
                   (RE_VAR('x'),
                    RE_VAR('y')),
                   {{"x",""},{"y",""}}) == Match_Ok);
    CHECK(evalSere(*RE_CONCAT
                   (RE_VAR('x'),
                    RE_VAR('y')),
                   {{"xy",""},{"x","y"}}) == Match_Failed);
    CHECK(evalSere
          (*RE_CONCAT
           (RE_CONCAT
            (RE_VAR('x'),
             RE_VAR('y')
             ),
            RE_VAR('x')),
           {{"x","y"}}) == Match_Partial);
    CHECK(evalSere
          (*RE_CONCAT
           (RE_CONCAT
            (RE_VAR('x'),
             RE_VAR('y')
             ),
            RE_VAR('x')),
           {{"x","y"},{"x","y"}}) == Match_Failed);
  }
}

TEST_CASE("Bool Expressions") {
  constexpr size_t atoms = 3;
  auto expr = GENERATE(Catch2::take(10, genBoolExpr(3, atoms)));
  auto letter = GENERATE(Catch2::take(10, genLetter(atoms)));

  SECTION("Z3") {
    CHECK(evalBool(*expr, letter) == evalBoolZ3(*expr, letter));
  }

  SECTION("RTP") {
    std::vector<uint8_t> data;
    rtp::toRTP(*expr, data);
    rtp::Names names;
    letterToBitSet(letter, names);
    CHECK(evalBool(*expr, letter) == rtp::eval(names, &data[0], data.size()));
  }
}

TEST_CASE("Sere vs Nfasl") {
  constexpr size_t atoms = 3;
  auto expr = GENERATE(Catch2::take(10, genBoolExpr(3, atoms)));
  auto word = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));
  auto nfasl = sereToNfasl(*expr);

  REQUIRE(evalSere(*expr, word) == evalNfasl(nfasl, word));
}

int main(int argc, char **argv) {
  // ::testing::InitGoogleTest(&argc, argv);
  // int result = RUN_ALL_TESTS();
  // if (result != 0) {
  //   return result;
  // }

  int result = Catch::Session().run( argc, argv );

  return result;
}

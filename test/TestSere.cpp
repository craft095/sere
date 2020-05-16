#include "catch2/catch.hpp"

#include "test/GenLetter.hpp"
#include "test/GenBoolExpr.hpp"
#include "test/EvalBoolExpr.hpp"
#include "test/EvalSere.hpp"
#include "test/EvalNfasl.hpp"

#include "test/Tools.hpp"
#include "test/ToolsZ3.hpp"

#include "ast/BoolExpr.hpp"
#include "ast/SereExpr.hpp"
#include "sat/Sat.hpp"

TEST_CASE("Sere") {
  SECTION("empty") {
    CHECK(evalSere(*RE_EMPTY, {}) == Match_Ok);
    CHECK(evalSere(*RE_EMPTY, {makeNames({},{})}) == Match_Failed);
  }
  SECTION("boolean") {
    CHECK(evalSere(*RE_SEREBOOL(RE_TRUE), {makeNames({1}, {0})}) == Match_Ok);
    CHECK(evalSere(*RE_SEREBOOL(RE_FALSE), {makeNames({1}, {0})}) == Match_Failed);
    CHECK(evalSere(*RE_SEREBOOL(RE_NOT(RE_TRUE)), {makeNames({1}, {0})}) == Match_Failed);
    CHECK(evalSere(*RE_SEREBOOL(RE_VAR(1)), {makeNames({1}, {0})}) == Match_Ok);
    CHECK(evalSere(*RE_SEREBOOL(RE_VAR(0)), {makeNames({1}, {0})}) == Match_Failed);

    Ptr<SereExpr> tc = RE_SEREBOOL(RE_AND(RE_TRUE, RE_NOT(RE_VAR(0))));
    CHECK(evalSere(*tc, {makeNames({1}, {0})}) == Match_Ok);
    CHECK(evalSere(*tc, {makeNames({0}, {1})}) == Match_Failed);
    CHECK(evalSere(*tc, {makeNames({1}, {0}), makeNames({1}, {0})}) == Match_Failed);
    CHECK(evalSere(*tc, {}) == Match_Partial);
    // ASSERT_EQ(evalSere(*tc, {{"z", "y"}}), Match_Failed);
  }

  SECTION("concat") {
    CHECK(evalSere(*RE_CONCAT
                   (RE_SEREBOOL(RE_VAR(0)),
                    RE_SEREBOOL(RE_VAR(1))),
                   {}) == Match_Partial);
    CHECK(evalSere(*RE_CONCAT
                   (RE_SEREBOOL(RE_VAR(0)),
                    RE_SEREBOOL(RE_VAR(1))),
                   {makeNames({0},{1}),makeNames({1},{0})}) == Match_Ok);
    CHECK(evalSere(*RE_CONCAT
                   (RE_SEREBOOL(RE_VAR(0)),
                    RE_SEREBOOL(RE_VAR(1))),
                   {makeNames({0,1},{}),makeNames({0},{1})}) == Match_Failed);
    CHECK(evalSere
          (*RE_CONCAT
           (RE_CONCAT
            (RE_SEREBOOL(RE_VAR(0)),
             RE_SEREBOOL(RE_VAR(1))
             ),
            RE_SEREBOOL(RE_VAR(0))),
           {makeNames({0},{1})}) == Match_Partial);
    CHECK(evalSere
          (*RE_CONCAT
           (RE_CONCAT
            (RE_SEREBOOL(RE_VAR(0)),
             RE_SEREBOOL(RE_VAR(1))
             ),
            RE_SEREBOOL(RE_VAR(0))),
           {makeNames({0},{1}),makeNames({0},{1})}) == Match_Failed);
  }
}

TEST_CASE("Bool Expressions") {
  constexpr size_t atoms = 3;

  CHECK(!sat(*RE_AND(
                     RE_VAR(0),
                     RE_NOT(RE_VAR(0)))));

  auto expr = GENERATE(Catch2::take(100, genBoolExpr(3, atoms)));

  SECTION("Minisat") {
    CHECK(sat(*expr) == satisfiable(boolSereToZex(*expr)));
  }
#if 0
  {
    auto letter = GENERATE(Catch2::take(10, genLetter(atoms)));

    SECTION("Z3") {
      CHECK(evalBool(*expr, letter) == evalBoolZ3(*expr, letter));
    }

    SECTION("RtPredicate") {
      std::vector<uint8_t> data;
      rt::toRtPredicate(*expr, data);
      CHECK(evalBool(*expr, letter) == rt::eval(letter, &data[0], data.size()));
    }
  }
#endif
}

TEST_CASE("Sere vs Nfasl") {
  constexpr size_t atoms = 3;
  auto expr = GENERATE(Catch2::take(10, genBoolExpr(3, atoms)));
  auto word = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));
  auto nfasl = sereToNfasl(*RE_SEREBOOL(expr));

  REQUIRE(evalSere(*RE_SEREBOOL(expr), word) == evalNfasl(nfasl, word));
}

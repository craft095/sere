#include "catch2/catch.hpp"

#include "test/GenExpr.hpp"
#include "test/EvalExpr.hpp"
#include "test/GenLetter.hpp"

#include "test/Letter.hpp"

#include "sat/Transform.hpp"
#include "sat/Simplify.hpp"
#include "sat/Sat.hpp"

TEST_CASE("boolean::Expr") {
  constexpr size_t atoms = 3;
  constexpr size_t depth = 5;

  auto expr = GENERATE(Catch2::take(1000, genExpr(depth, atoms)));
  auto letter = GENERATE(Catch2::take(32, genLetter(atoms)));

  bool r0 = evalBool(expr, letter);
  bool r1 = evalBool(nnf(expr), letter);
  bool r2 = evalBool(simplify(expr), letter);

  CHECK(r0 == r1);
  CHECK(r0 == r2);
}

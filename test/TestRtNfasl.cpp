#include "catch2/catch.hpp"

#include "GenBoolExpr.hpp"
#include "GenNfasl.hpp"
#include "GenLetter.hpp"
#include "EvalBoolExpr.hpp"
#include "EvalNfasl.hpp"
#include "EvalRtNfasl.hpp"

#include "Tools.hpp"
#include "ToolsZ3.hpp"

#include "BisimNfasl.hpp"
#include "Letter.hpp"

using namespace nfasl;

TEST_CASE("RtNfasl") {
  constexpr size_t atoms = 2;
  constexpr size_t states = 4;
  constexpr size_t maxTrs = 2;
  constexpr size_t depth = 3;

  auto expr0 = GENERATE(Catch2::take(100, genNfasl(depth, atoms, states, maxTrs)));
  auto word0 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

  Nfasl expr1;
  clean(*expr0, expr1);

  Match r0 = evalCleanNfasl(expr1, word0);

  rt::Nfasl rtNfasl;
  toRt(expr1, rtNfasl);

  Match r1 = evalRtNfasl(rtNfasl, word0);

  CHECK(r0 == r1);
}

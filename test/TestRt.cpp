#include "catch2/catch.hpp"

#include "test/GenBoolExpr.hpp"
#include "test/GenDfasl.hpp"
#include "test/GenNfasl.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalBoolExpr.hpp"
#include "test/EvalDfasl.hpp"
#include "test/EvalNfasl.hpp"
#include "test/EvalRt.hpp"

#include "test/Tools.hpp"
#include "test/ToolsZ3.hpp"
#include "test/Letter.hpp"

#include "nfasl/BisimNfasl.hpp"

using namespace nfasl;
using namespace dfasl;

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

TEST_CASE("RtDfasl") {
  constexpr size_t atoms = 2;
  constexpr size_t states = 4;
  constexpr size_t maxTrs = 2;
  constexpr size_t depth = 3;

  auto expr0 = GENERATE(Catch2::take(100, genNfasl(depth, atoms, states, maxTrs)));
  auto word0 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

  Nfasl expr1;
  clean(*expr0, expr1);

  Match r0 = evalCleanNfasl(expr1, word0);

  Dfasl expr2;
  rt::Dfasl rtDfasl;
  toDfasl(expr1, expr2);
  toRt(expr2, rtDfasl);

  Match r1 = evalRtDfasl(rtDfasl, word0);

  CHECK(r0 == r1);
}

#include "catch2/catch.hpp"

#include "test/GenBoolExpr.hpp"
#include "test/GenNfasl.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalBoolExpr.hpp"
#include "test/EvalNfasl.hpp"
#include "test/EvalDfasl.hpp"

#include "test/Tools.hpp"
#include "test/Letter.hpp"

#include "nfasl/BisimNfasl.hpp"

using namespace nfasl;
using namespace dfasl;

TEST_CASE("Nfasl to Dfasl") {
  constexpr size_t atoms = 4;
  constexpr size_t depth = 4;
  constexpr size_t states = 4;
  constexpr size_t maxTrs = 4;

  auto expr0 = GENERATE(Catch2::take(100, genNfasl(depth, atoms, states, maxTrs)));
  auto word0 = GENERATE(Catch2::take(5, genWord(atoms, 0, 5)));

  Nfasl cleaned;
  clean(*expr0, cleaned);

  Dfasl dfa;
  toDfasl(cleaned, dfa);

  Match r0 = evalCleanNfasl(cleaned, word0);
  Match r1 = evalCleanDfasl(dfa, word0);

  CHECK(r0 == r1);
}

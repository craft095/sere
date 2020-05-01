#include "catch2/catch.hpp"

#include "test/GenBoolExpr.hpp"
#include "test/GenNfasl.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalBoolExpr.hpp"
#include "test/EvalNfasl.hpp"

#include "test/Tools.hpp"
#include "test/ToolsZ3.hpp"

#include "test/Letter.hpp"

#include "nfasl/BisimNfasl.hpp"

using namespace nfasl;

TEST_CASE("Nfasl") {
  State s0{0}, s1{1};
  TransitionRule r00 = { Predicate::var(0),
                         s1 };
  TransitionRule r10 = { !Predicate::var(1),
                         s0 };
  Nfasl a = {
    2,  // atomic count
    2,  // state count
    s0, // initial
    { s1 }, // finals
    { { r00 }, { r10 } } // transitions
  };

  CHECK(evalNfasl(a, {}) == Match_Partial);
  CHECK(evalNfasl(a, {makeNames({}, {0,1})}) == Match_Failed);
  CHECK(evalNfasl(a, {makeNames({0}, {1})}) == Match_Ok);
  CHECK(evalNfasl(a, {makeNames({0}, {1}), makeNames({0}, {1})}) == Match_Partial);
}

TEST_CASE("Nfasl, minimal") {
  State s0{0}, s1{1}, s2{2}, s3{3}, s4{4};
  TransitionRule r01 = { Predicate::var(0),
                         s1 };
  TransitionRule r03 = { Predicate::var(0),
                         s3 };
  TransitionRule r12 = { !Predicate::var(1),
                         s2 };
  TransitionRule r34 = { !Predicate::var(1),
                         s4 };
  Nfasl a = {
    2,  // atomic count
    5,  // state count
    s0, // initial
    { s2, s4 }, // finals
    { { r01, r03 },
      { r12 },
      {},
      { r34 },
      {}
    } // transitions
  };

  Nfasl b;
  minimize(a, b);

  CHECK(b.stateCount == 3);
  CHECK(b.finals.size() == 1);

  CHECK(evalNfasl(b, {}) == Match_Partial);
  CHECK(evalNfasl(b, {makeNames({}, {0,1})}) == Match_Failed);
  CHECK(evalNfasl(b, {makeNames({0}, {1})}) == Match_Partial);
  CHECK(evalNfasl(b, {makeNames({0}, {1}), makeNames({0}, {1})}) == Match_Ok);
  CHECK(evalNfasl(b, {makeNames({0}, {1}), makeNames({0}, {1}), makeNames({0}, {1})}) == Match_Failed);
}

#define CHECK_IF(premise, cond) if ((premise)) { CHECK(cond); }

TEST_CASE("Nfasl, operations") {
  constexpr size_t atoms = 2;
  constexpr size_t states = 3;
  constexpr size_t maxTrs = 2;

  auto expr0 = GENERATE(Catch2::take(10, genNfasl(3, atoms, states, maxTrs)));
  auto word0 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

  Match r0 = evalNfasl(*expr0, word0);

  SECTION("clean") {
    Nfasl cleaned;
    clean(*expr0, cleaned);
    Match r1 = evalCleanNfasl(cleaned, word0);
    // *expr may conatin partial execution traces
    // which have no chance to succeed, because
    // final states are not always reachable.
    // so it is required to exclude such cases
    if (r0 == Match_Partial) {
      CHECK(r1 != Match_Ok);
    } else {
      CHECK(r0 == r1);
    }

    SECTION("partial") {
      Nfasl part = partial(cleaned);
      Match r2 = evalCleanNfasl(part, word0);
      if (r1 == Match_Failed) {
        CHECK(r2 == Match_Failed);
      } else {
        CHECK(r2 == Match_Ok);
      }
    }
  }

  SECTION("minimize") {
    Nfasl cleaned, minimal;
    clean(*expr0, cleaned);
    minimize(*expr0, minimal);

    Match r0 = evalNfasl(cleaned, word0);
    Match r1 = evalNfasl(minimal, word0);

    CHECK(r0 == r1);

    SECTION("complement") {
      Nfasl comp;
      complement(minimal, comp);
      Match r2 = evalCleanNfasl(comp, word0);
      if (r1 != Match_Ok) {
        CHECK(r2 == Match_Ok);
      } else {
        CHECK(r2 != Match_Ok);
      }
      if (r2 != Match_Ok) {
        CHECK(r1 == Match_Ok);
      } else {
        CHECK(r1 != Match_Ok);
      }
    }
  }

  SECTION("unary ops") {
    auto wordMult2 {word0};
    wordMult2.insert(wordMult2.end(), word0.begin(), word0.end());

    SECTION("kleene star") {
      Match r_1 = evalNfasl(kleeneStar(*expr0), word0);
      if (word0.size() == 0) {
        CHECK(r_1 == Match_Ok);
      } else if (r0 == Match_Ok) {
        Match r_2 = evalNfasl(kleeneStar(*expr0), wordMult2);
        CHECK(r_1 == Match_Ok);
        CHECK(r_2 == Match_Ok);
      }
    }

    SECTION("kleene plus") {
      if (r0 == Match_Ok) {
        Match r_1 = evalNfasl(kleenePlus(*expr0), word0);
        Match r_2 = evalNfasl(kleeneStar(*expr0), wordMult2);
        CHECK(r_1 == Match_Ok);
        CHECK(r_2 == Match_Ok);
      }
    }
  }

  SECTION("binary ops") {
    auto expr1 = GENERATE(Catch2::take(10, genNfasl(2, atoms, states, maxTrs)));

#if 1
    SECTION("intersect/union") {
      Match r1 = evalNfasl(*expr1, word0);

      SECTION("intersect") {
        Match r = evalNfasl(intersects(*expr0, *expr1), word0);

        if (r0 == Match_Ok && r1 == Match_Ok) {
          CHECK(r == Match_Ok);
        } else if (r0 == Match_Failed || r1 == Match_Failed) {
          CHECK(r == Match_Failed);
        } else {
          CHECK(r == Match_Partial);
        }
      }

      SECTION("union") {
        Match r = evalNfasl(unions(*expr0, *expr1), word0);

        if (r0 == Match_Ok || r1 == Match_Ok) {
          CHECK(r == Match_Ok);
        } else if (r0 == Match_Failed && r1 == Match_Failed) {
          CHECK(r == Match_Failed);
        } else {
          CHECK(r == Match_Partial);
        }
      }
    }
#endif
    SECTION("concat") {
      // auto word0 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

#if 1
      SECTION("concat") {
        auto word1 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));
        auto word{word0};
        word.insert(word.end(), word1.begin(), word1.end());

        Match r1 = evalNfasl(*expr1, word1);

        Match r = evalNfasl(concat(*expr0, *expr1), word);

        if (r0 == Match_Ok && r1 == Match_Ok) {
          CHECK(r == Match_Ok);
        }
      }

      SECTION("fuse") {
        auto word1 = GENERATE(Catch2::take(3, genWord(atoms, 1, 2)));

        if (word0.size() > 0 && word1.size() > 0) {
          auto word{word0};
          word1[0] = word0[word0.size() - 1];
          word.insert(word.end(), word1.begin() + 1, word1.end());

          Match r1 = evalNfasl(*expr1, word1);
          Match r = evalNfasl(fuse(*expr0, *expr1), word);

          if (r0 == Match_Ok && r1 == Match_Ok) {
            CHECK(r == Match_Ok);
          }
        }
      }
#endif
    }
  }
}

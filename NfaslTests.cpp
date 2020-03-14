#include <iostream>
#include <memory>

#define CATCH_CONFIG_RUNNER
#include "Nfasl.hpp"
#include "Letter.hpp"
#include "EvalSere.hpp"
#include "TestLetter.hpp"
#include "Algo.hpp"

using namespace nfasl;

Match evalNfasl(const Nfasl& a, const Word& word) {
  States curr, next;

  curr.insert(a.initial);
  next.insert(a.initial);

  for (auto const& letter : word) {
    next.clear();
    z3::expr lexpr = letterToZex(letter);
    for (auto const& s : curr) {
      for (auto const& rule : a.transitions[s]) {
        if (evalWithImply(lexpr, rule.phi)) {
          next.insert(rule.state);
        }
      }
    }
    std::swap(curr, next);
  }

  if (curr.empty()) {
    return Match_Failed;
  }
  if (set_non_empty_intersection(curr, a.finals)) {
    return Match_Ok;
  } else {
    return Match_Partial;
  }
}

class NfaslGenerator : public Catch2::IGenerator <Ptr<Nfasl>> {
public:
  NfaslGenerator(size_t depth, size_t atoms_, size_t states_, size_t maxTrs_)
    : maxDepth(depth), atoms(atoms_), states(states_), maxTrs(maxTrs_) {
    next();
  }

  Ptr<Nfasl> const& get() const override {
    return value;
  }

  bool next() override {
    value = makeNfasl(maxDepth, atoms, states, maxTrs);
    return true;
  }

private:
  size_t maxDepth;
  size_t atoms;
  size_t states;
  size_t maxTrs;
  Ptr<Nfasl> value;
};

inline Catch2::GeneratorWrapper<Ptr<Nfasl>>
genNfasl(size_t depth, size_t atomics, size_t states, size_t maxTrs) {
  return Catch2::GeneratorWrapper<Ptr<Nfasl>>
    (std::make_unique<NfaslGenerator>
     (depth, atomics, states, maxTrs));
}

TEST_CASE("Nfasl") {
  State s0{0}, s1{1};
  TransitionRule r00 = { boolSereToZex(*RE_VAR("a")),
                         s1 };
  TransitionRule r10 = { boolSereToZex(*RE_NOT(RE_VAR("b"))),
                         s0 };
  Nfasl a = {
    { "a", "b" },
    2,  // atomic count
    2,  // state count
    s0, // initial
    { s1 }, // finals
    { { r00 }, { r10 } } // transitions
  };

  CHECK(evalNfasl(a, {}) == Match_Partial);
  CHECK(evalNfasl(a, {{"", "ab"}}) == Match_Failed);
  CHECK(evalNfasl(a, {{"a", "b"}}) == Match_Ok);
  CHECK(evalNfasl(a, {{"a", "b"}, {"a", "b"}}) == Match_Partial);
}

#define CHECK_IF(premise, cond) if ((premise)) { CHECK(cond); }

TEST_CASE("Nfasl, operations") {
  constexpr size_t atoms = 2;
  constexpr size_t states = 3;
  constexpr size_t maxTrs = 2;

  auto expr0 = GENERATE(Catch2::take(10, genNfasl(3, atoms, states, maxTrs)));
  auto expr1 = GENERATE(Catch2::take(10, genNfasl(2, atoms, states, maxTrs)));

  SECTION("intersect/union") {
    auto word = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

    Match r0 = evalNfasl(*expr0, word);
    Match r1 = evalNfasl(*expr1, word);

    SECTION("intersect") {
      Match r = evalNfasl(intersects(*expr0, *expr1), word);

      if (r0 == Match_Ok && r1 == Match_Ok) {
        CHECK(r == Match_Ok);
      } else if (r0 == Match_Failed || r1 == Match_Failed) {
        CHECK(r == Match_Failed);
      } else {
        CHECK(r == Match_Partial);
      }
    }

    SECTION("union") {
      Match r = evalNfasl(unions(*expr0, *expr1), word);

      if (r0 == Match_Ok || r1 == Match_Ok) {
        CHECK(r == Match_Ok);
      } else if (r0 == Match_Failed && r1 == Match_Failed) {
        CHECK(r == Match_Failed);
      } else {
        CHECK(r == Match_Partial);
      }
    }
  }
  SECTION("concat") {
    auto word0 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));
    auto word1 = GENERATE(Catch2::take(3, genWord(atoms, 0, 3)));

    Match r0 = evalNfasl(*expr0, word0);
    Match r1 = evalNfasl(*expr1, word1);

    auto word{word0};
    word.insert(word.end(), word1.begin(), word1.end());

    Match r = evalNfasl(concat(*expr0, *expr1), word);

    if (r0 == Match_Ok && r1 == Match_Ok) {
      CHECK(r == Match_Ok);
    }
  }
}

int main(int argc, char **argv) {
  int result = Catch::Session().run( argc, argv );
  return result;
}

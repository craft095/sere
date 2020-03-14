#include <iostream>
#include <memory>

#define CATCH_CONFIG_RUNNER
#include "Nfasl.hpp"
#include "Letter.hpp"
#include "EvalSere.hpp"
#include "Algo.hpp"

using namespace nfasl;

Match evalNfasl(Nfasl& a, const Word& word) {
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

int main(int argc, char **argv) {
  int result = Catch::Session().run( argc, argv );
  return result;
}

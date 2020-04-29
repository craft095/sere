#include "test/EvalExpr.hpp"
#include "test/EvalDfasl.hpp"

#include "Algo.hpp"
#include "nfasl/Dfasl.hpp"
#include "test/Letter.hpp"

using namespace dfasl;

/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 */
Match evalDfasl(const Dfasl& a, const Word& word) {
  constexpr State nostate = std::numeric_limits<State>::max();
  State curr, next;

  curr = a.initial;
  next = a.initial;

  for (auto const& letter : word) {
    next = nostate;
    for (auto const& rule : a.transitions[curr]) {
      if (evalBool(rule.phi, letter)) {
        next = rule.state;
        break;
      }
    }
    if (next == nostate) {
      return Match_Failed;
    }
    curr = next;
  }

  if (set_member(a.finals, curr)) {
    return Match_Ok;
  } else {
    return Match_Partial;
  }
}

Match evalCleanDfasl(const dfasl::Dfasl& a, const Word& word) {
  // if automaton is cleaned, it is enough to be sure
  // that it fails on any input
  if (a.finals.empty()) {
    return Match_Failed;
  }
  return evalDfasl(a, word);
}

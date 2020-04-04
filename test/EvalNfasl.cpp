#include "EvalBoolExpr.hpp"
#include "EvalNfasl.hpp"

#include "Algo.hpp"
#include "Nfasl.hpp"
#include "Letter.hpp"

using namespace nfasl;

/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 */
Match evalNfasl(const Nfasl& a, const Word& word) {
  States curr, next;

  curr.insert(a.initial);
  next.insert(a.initial);

  for (auto const& letter : word) {
    next.clear();
    for (auto const& s : curr) {
      for (auto const& rule : a.transitions[s]) {
        if (evalBool(*rule.phi, letter)) {
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

Match evalCleanNfasl(const nfasl::Nfasl& a, const Word& word) {
  // if automaton is cleaned, it is enough to be sure
  // that it fails on any input
  if (a.finals.empty()) {
    return Match_Failed;
  }
  return evalNfasl(a, word);
}

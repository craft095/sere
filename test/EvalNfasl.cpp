#include "EvalBoolExpr.hpp"
#include "EvalNfasl.hpp"

#include "Algo.hpp"
#include "Nfasl.hpp"
#include "Letter.hpp"

using namespace nfasl;

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

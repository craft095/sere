#include "test/EvalExpr.hpp"
#include "test/EvalNfasl.hpp"

#include "Algo.hpp"
#include "nfasl/Nfasl.hpp"
#include "test/Letter.hpp"

#include <map>

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
        if (evalBool(rule.phi, letter)) {
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

struct RtContext {
  static constexpr size_t NoValue = std::numeric_limits<size_t>::max();

  size_t longest;
  size_t shortest;

  static RtContext advance(RtContext ctx) {
    if (ctx.defined()) {
      ++ctx.longest;
      ++ctx.shortest;
    } else {
      ctx.longest = 0;
      ctx.shortest = 0;
    }
    return ctx;
  }

  RtContext() : longest(NoValue), shortest(NoValue) {}

  bool defined() const {
    return longest != NoValue && shortest != NoValue;
  }

  void started() {
    if (longest == NoValue) {
      longest = 0;
    }
    shortest = 0;
  }
  void merge(const RtContext& ctx) {
    if (defined() && ctx.defined()) {
      longest = std::max(longest, ctx.longest);
      shortest = std::min(shortest, ctx.shortest);
    } else if (ctx.defined()) {
      *this = ctx;
    }
  }
};

/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 * Note: it does not produce FAILED at all
 */
ExtendedMatch evalExtendedNfasl(const Nfasl& a, const Word& word) {
  typedef std::map<State, RtContext> StateMap;

  StateMap currCtx, nextCtx;
  States curr, next;

  curr.insert(a.initial);
  currCtx[a.initial].started();

  for (size_t time = 0; time < word.size(); ++time) {
    auto const& letter = word[time];
    next.clear();
    next.insert(a.initial);
    nextCtx.clear();
    nextCtx[a.initial].started();
    for (auto const& s : curr) {
      for (auto const& rule : a.transitions[s]) {
        if (evalBool(rule.phi, letter)) {
          nextCtx[rule.state].merge(RtContext::advance(currCtx[s]));
          next.insert(rule.state);
        }
      }
    }
    std::swap(curr, next);
    std::swap(currCtx, nextCtx);
  }

  size_t horizon = 0;
  RtContext horizonCtx;
  horizonCtx.started();
  for (auto const& v : currCtx) {
    horizonCtx.merge(v.second);
  }
  assert(horizonCtx.defined());
  horizon = horizonCtx.longest;

  ExtendedMatch match;
  if (set_non_empty_intersection(curr, a.finals)) {
    // fold currCtx over final states
    RtContext ctx;
    for (auto s : a.finals) {
      ctx.merge(currCtx[s]);
    }
    assert(ctx.defined());
    match.match = Match_Ok;
    match.ok.shortest = ctx.shortest;
    match.ok.longest = ctx.longest;
    match.ok.horizon = horizon;
  } else {
    match.match = Match_Partial;
    match.partial.horizon = horizon;
  }
  return match;
}

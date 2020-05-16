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
    assert(ctx.defined());
    if (defined()) {
      longest = std::max(longest, ctx.longest);
      shortest = std::min(shortest, ctx.shortest);
    } else {
      *this = ctx;
    }
  }
};

/**
 * The result of a SERE evaluation. There are three
 * possible outcomes: ok, partial & failed
 * Extended means that it also provide information about
 * which event subsequence matches the SERE (it is always a suffix of a word/stream)
 *
 * `failed` is flagged with Match_Failed (field `match`)
 * and is not supplied with additional information
 * As we find a matching stream suffix, `failed` can only be
 * returned if SERE is equivalent to FALSE: i.e. there is no
 * seqence matching the SERE (even potentially).
 *
 * `partial` is flagged with Match_Partial and
 * there is a `paritial.horizon` - length of a longest stream suffix
 * that (potentially) starts a matching subsequence.
 * Client code may use this information, for example, to discard
 * any events ealier then `paritial.horizon`
 *
 * `ok` is flagged with Match_Ok and it comes with the following
 * information:
 * `ok.horizon` - the same meaning as in `partial.horizon`
 * `ok.longest` - length of a longest match
 * `ok.shortest` - length of a shortest match
 *
 * Note, that longest/shortest are with regard to current stream
 * state. Streams are potentially infinite, but SERE library is only
 * dealing with finite stream prefix from start to stream head.
 *
 * Example:
 *   ((B;A) | (B;B;B)[*] | (B[*] ; A ; A))
 *
 *   stream: [] --> partial (horizon = 0)
 *   stream: [B] --> partial (horizon = 1)
 *   stream: [B,B] --> partial (horizon = 2)
 *   stream: [B,B,B] --> ok (longest=3, shortest=3, horizon = 3)
 *   stream: [B,B,B,A] --> ok (longest=2, shortest=2, horizon = 4)
 *   stream: [B,B,B,A,C] --> partial (horizon = 0)
 *   stream: [B,B,B,A,C,B] --> partial (horizon = 1)
 */
struct ExtendedMatch {
  Match match;
  union {
    struct {
      size_t longest; /** the ealiest event starting a matched subsequence */
      size_t shortest; /** the latest event starting a matched subsequence */
      size_t horizon; /** the ealiest event to find a match */
    } ok;
    struct {
      size_t horizon; /** the ealiest event to find a match */
    } partial;
  };
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
    nextCtx.clear();
    nextCtx[a.initial].started();
    for (auto const& s : curr) {
      for (auto const& rule : a.transitions[s]) {
        if (evalBool(rule.phi, letter)) {
          nextCtx[rule.state].merge(currCtx[rule.state]);
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

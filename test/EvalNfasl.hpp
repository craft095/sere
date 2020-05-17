#ifndef EVALNFASL_HPP
#define EVALNFASL_HPP

#include "Match.hpp"
#include "nfasl/Nfasl.hpp"
#include "test/Letter.hpp"

inline bool operator==(const ExtendedMatch& u, const ExtendedMatch& v) {
  bool r = u.match == v.match;
  switch(u.match) {
  case Match_Ok:
    r &= u.ok.shortest == v.ok.shortest;
    r &= u.ok.longest == v.ok.longest;
    r &= u.ok.horizon == v.ok.horizon;
    break;
  case Match_Partial:
    r &= u.partial.horizon == v.partial.horizon;
    break;
  }
  return r;
}

/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 */
extern Match evalNfasl(const nfasl::Nfasl& a, const Word& word);
/**
 * This evaluator correctly handles every case.
 * But it is required cleaned automaton, i.e. with unreachable
 * states removed
 */
extern Match evalCleanNfasl(const nfasl::Nfasl& a, const Word& word);
/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 * Note: it does not produce FAILED at all
 */
extern ExtendedMatch evalExtendedNfasl(const nfasl::Nfasl& a, const Word& word);

#endif // EVALNFASL_HPP

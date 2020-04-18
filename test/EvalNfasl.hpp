#ifndef EVALNFASL_HPP
#define EVALNFASL_HPP

#include "Match.hpp"
#include "nfasl/Nfasl.hpp"
#include "test/Letter.hpp"

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

#endif // EVALNFASL_HPP

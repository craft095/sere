#ifndef EVALDFASL_HPP
#define EVALDFASL_HPP

#include "Match.hpp"
#include "nfasl/Dfasl.hpp"
#include "test/Letter.hpp"

/**
 * This evaluator may produce PARTIAL results
 * even if there is no chance to match the word.
 *
 * It may happen due to presense of unreachable states.
 */
extern Match evalDfasl(const dfasl::Dfasl& a, const Word& word);
/**
 * This evaluator correctly handles every case.
 * But it is required cleaned automaton, i.e. with unreachable
 * states removed
 */
extern Match evalCleanDfasl(const dfasl::Dfasl& a, const Word& word);

#endif // EVALDFASL_HPP

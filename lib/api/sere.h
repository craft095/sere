#ifndef API_SERE_H
#define API_SERE_H

#include <stdint.h>

#include "Match.hpp"

#define MATCH_OK Match_Ok /** sequence matches */
#define MATCH_PARTIAL Match_Partial /** sequence has a matching continuation */
#define MATCH_FAILED Match_Failed /** sequence does not match */

#define SERE_FORMAT_JSON 0 /** JSON target automaton format */
#define SERE_FORMAT_RT 1 /** Binary target automaton format */

#define SERE_TARGET_NFASL 0 /** NFASL target */
#define SERE_TARGET_DFASL 1 /** DFASL target */

struct sere_ref;
struct sere_context;

/**
 * Options to control resource consumption
 */
struct sere_options {
  int target; /** target automata (SERE_TARGET_NFASL or SERE_TARGET_DFASL) */
  int format; /** target format, only SERE_FORMAT_JSON supported */
  size_t maxNfaslStates; /** abort if number of NFASL states exceeds the limit */
  size_t maxDfaslStates; /** abort if number of DFASL states exceeds the limit */
};

/**
 * SERE compilation results
 */
struct sere_compiled {
  int compiled; /** non-zero if compiled, zero in case of error */
  struct sere_ref* ref; /** opaque reference */
  union {
    struct {
      const char* error; /** compilation error */
    };
    struct {
      const char* content; /** serialized *FASL */
      size_t content_size; /** serialized *FASL size */
    };
  };
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compile SERE expression
 *
 * @param[in] expr NUL terminated SERE expression
 * @param[in] opts options to control compilation
 * @param[out] result compilation results
 * @returns non-zero in case of errors
 */
int sere_compile(const char* expr,
                 const struct sere_options* opts,
                 struct sere_compiled* result);

/**
 * Release SERE compilation results
 *
 * @param[in] compiled compilation results
 */
void sere_release(struct sere_compiled* compiled);

/**
 * Load compiled SERE expression
 *
 * @param[in] rt SERE image
 * @param[in] rt_size SERE image size
 * @param[out] sere loaded SERE
 * @returns non-zero in case of errors
 */
int sere_context_load(const char* rt, /** serialized *FASL */
                      size_t rt_size, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      );

/**
 * Release resources, allocated for SERE
 *
 * @param[in] sere SERE context
 */
void sere_context_release(void* sere);

/**
 * Get number of atomic predicates in SERE
 *
 * @param[in] sere SERE context
 * @param[out] count number of atomic predicates
 */
void sere_context_atomic_count(void* sere, size_t* count);

/**
 * Get name of a given atomic predicate
 *
 * All predicates are numbered from zero to `count`-1,
 * where `count` can be requested with `sere_context_atomic_count`
 *
 * @param[in] sere SERE context
 * @param[in] id atomic predicate id
 * @param[out] name predicate name
 * @returns non-zero in case of errors
 */
int sere_context_atomic_name(void* sere, size_t id, const char** name);

/**
 * Reset SERE context to its initial state
 *
 * @param[in] sere SERE context
 */
void sere_context_reset(void* sere);

/**
 * Set atomic predicate to TRUE
 *
 * All predicates are set to FALSE at every step.
 *
 * @param[in] sere SERE context
 * @param[in] id atomic predicate to set
 * @returns non-zero in case of error (incorrect predicate id)
 */
int sere_context_set_atomic(void* sere, size_t id);

/**
 * Advance SERE's automaton
 *
 * @param[in] sere SERE context
 */
void sere_context_advance(void* sere);

/**
 * Get match results.
 *
 * @param[in] sere SERE context
 * @param[out] result match result
 */
void sere_context_get_result(void* sere, int* result);


/**
 * Load compiled SERE expression
 *
 * @param[in] rt SERE image
 * @param[in] rt_size SERE image size
 * @param[out] sere loaded extended SERE context
 * @returns non-zero in case of errors
 */
int sere_context_extended_load(const char* rt, /** serialized *FASL */
                               size_t rt_size, /** serialized *FASL size */
                               void** sere /** loaded extended SERE context */
                               );

/**
 * Release resources, allocated for SERE
 *
 * @param[in] sere SERE context
 */
void sere_context_extended_release(void* sere);

/**
 * Get number of atomic predicates in SERE
 *
 * @param[in] sere SERE context
 * @param[out] count number of atomic predicates
 */
void sere_context_extended_atomic_count(void* sere, size_t* count);

/**
 * Get name of a given atomic predicate
 *
 * All predicates are numbered from zero to `count`-1,
 * where `count` can be requested with `sere_context_atomic_count`
 *
 * @param[in] sere SERE context
 * @param[in] id atomic predicate id
 * @param[out] name predicate name
 * @returns non-zero in case of errors
 */
int sere_context_extended_atomic_name(void* sere, size_t id, const char** name);

/**
 * Reset SERE context to its initial state
 *
 * @param[in] sere SERE context
 */
void sere_context_extended_reset(void* sere);

/**
 * Set atomic predicate to TRUE
 *
 * All predicates are set to FALSE at every step.
 *
 * @param[in] sere SERE context
 * @param[in] id atomic predicate to set
 * @returns non-zero in case of error (incorrect predicate id)
 */
int sere_context_extended_set_atomic(void* sere, size_t id);

/**
 * Advance SERE's automaton
 *
 * @param[in] sere SERE context
 */
void sere_context_extended_advance(void* sere);

/**
 * Get match results.
 *
 * @param[in] sere SERE context
 * @param[out] result match result
 */
void sere_context_extended_get_result(void* sere, struct ExtendedMatch* result);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // API_SERE_H

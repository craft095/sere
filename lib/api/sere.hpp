#ifndef API_SERE_HPP
#define API_SERE_HPP

#include <cstddef>
#include <cstdint>

#include "Match.hpp"

#define MATCH_OK Match_Ok
#define MATCH_PARTIAL Match_Partial
#define MATCH_FAILED Match_Failed

struct sere_ref;
struct sere_context;

/**
 * Options to control resource consumption
 */
struct sere_options {
  size_t maxNfaslStates; /** abort if number of NFASL states exceeds the limit */
  size_t maxDfaslStates; /** abort if number of DFASL states exceeds the limit */
};

/**
 * SERE compilation results
 */
struct sere_compiled {
  int compiled;
  union {
    struct {
      int r;
    };
    struct {
      struct sere_ref* ref; /** opaque reference */
      const uint8_t* rt; /** serialized *FASL */
      size_t rt_size; /** serialized *FASL size */
      const char** atomics; /** atomic predicate names */
      size_t atomics_count; /** atomic predicate names count */
    };
  };
};

/**
 * Compile SERE expression
 *
 * @param[in] expr NUL terminated SERE expression
 * @param[in] opts options to control compilation
 * @param[out] result compilation results
 */
extern "C"
int sere_compile(const char* expr,
                 const struct sere_options* opts,
                 sere_compiled* result);

/**
 * Release SERE compilation results
 */
extern "C"
void sere_release(sere_compiled* result);

extern "C"
int sere_context_load(const uint8_t* rt, /** serialized *FASL */
                      size_t rt_size, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      );
extern "C"
void sere_context_release(void* sere);
extern "C"
void sere_context_reset(void* sere);
extern "C"
void sere_context_advance(void* sere,
                          const char* atomics,
                          size_t atomics_count);
extern "C"
void sere_context_get_result(void* sere, int* result);

#endif // API_SERE_HPP

#ifndef API_SERE_HPP
#define API_SERE_HPP

#include <cstddef>
#include <cstdint>
#include "sere.h"

#if 0
#include <cstddef>
#include <cstdint>

#include "Match.hpp"

#define MATCH_OK Match_Ok
#define MATCH_PARTIAL Match_Partial
#define MATCH_FAILED Match_Failed

#define SERE_FORMAT_JSON 0
#define SERE_FORMAT_RT 1

#define SERE_TARGET_NFASL 0
#define SERE_TARGET_DFASL 1

struct sere_ref;
struct sere_context;

/**
 * Options to control resource consumption
 */
struct sere_options {
  int target;
  int format;
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
      const char* content; /** serialized *FASL */
      size_t content_size; /** serialized *FASL size */
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
int sere_context_load(const char* rt, /** serialized *FASL */
                      size_t rt_size, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      );
extern "C"
void sere_context_atomic_count(void* ctx, size_t* count);
extern "C"
void sere_context_atomic_name(void* ctx, size_t id, const char** name);
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
#endif // 0
#endif // API_SERE_HPP

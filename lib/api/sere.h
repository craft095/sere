#ifndef API_SERE_H
#define API_SERE_H

#include <stdint.h>

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
 */
int sere_compile(const char* expr,
                 const struct sere_options* opts,
                 struct sere_compiled* result);

/**
 * Release SERE compilation results
 */
void sere_release(struct sere_compiled* result);

int sere_context_load(const char* rt, /** serialized *FASL */
                      size_t rt_size, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      );
void sere_context_atomic_count(void* ctx, size_t* count);
void sere_context_atomic_name(void* ctx, size_t id, const char** name);
void sere_context_release(void* sere);
void sere_context_reset(void* sere);
void sere_context_advance(void* sere,
                          const char* atomics,
                          size_t atomics_count);
void sere_context_get_result(void* sere, int* result);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // API_SERE_H

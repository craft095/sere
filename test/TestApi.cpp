#include "catch2/catch.hpp"

#include "api/sere.hpp"

#include <string>
#include <memory.h>

TEST_CASE("Sere API") {
  //const char expr[] = "(A ; B[*] ; ~(C | D) ; E) & F[+]";
  const char expr[] = "(A ; B[*]) & F[+]";

  struct sere_options opts = { SERE_TARGET_DFASL, SERE_FORMAT_JSON, 0, 0 };
  struct sere_compiled compiled;
  int r = sere_compile(expr, &opts, &compiled);

  CHECK(r == 0);

  void* sere = nullptr;
  r = sere_context_load(compiled.content, compiled.content_size, &sere);

  CHECK(r == 0);

  size_t atomic_count;
  sere_context_atomic_count(sere, &atomic_count);

  char atomics[atomic_count];
  auto mapper = [&sere, &atomic_count, &atomics](char c) {
                  for (size_t ix = 0; ix < atomic_count; ++ix) {
                    const char* name = nullptr;
                    sere_context_atomic_name(sere, ix, &name);
                    assert(name != 0);
                    if (name[0] == c) {
                      atomics[ix] = 1;
                    }
                  }
                };

  std::string word[] = { "AF", "ABF", "BBF" };

  int result;
  sere_context_get_result(sere, &result);
  CHECK(result == MATCH_PARTIAL);
  for (size_t il = 0; il < sizeof(word)/sizeof(word[0]); ++il) {
    memset((void*)atomics, 0, sizeof(atomics));;
    for (auto s : word[il]) {
      mapper(s);
    }

    sere_context_advance(sere, atomics, sizeof(atomics));

    sere_context_get_result(sere, &result);
    CHECK(result == MATCH_OK);
  }
  sere_context_release(sere);
  sere_release(&compiled);
}

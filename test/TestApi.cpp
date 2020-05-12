#include "catch2/catch.hpp"

#include "api/sere.hpp"

#include <map>
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

  std::map<char, size_t> remap;

  for (size_t ix = 0; ix < atomic_count; ++ix) {
    const char* name = nullptr;
    sere_context_atomic_name(sere, ix, &name);
    remap[name[0]] = ix;
  }

  std::string word[] = { "AF", "ABF", "BBF" };

  int result;
  sere_context_get_result(sere, &result);
  CHECK(result == MATCH_PARTIAL);
  for (size_t il = 0; il < sizeof(word)/sizeof(word[0]); ++il) {
    for (auto s : word[il]) {
      sere_context_set_atomic(sere, remap[s]);
    }
    sere_context_advance(sere);
    sere_context_get_result(sere, &result);
    CHECK(result == MATCH_OK);
  }
  sere_context_release(sere);
  sere_release(&compiled);
}

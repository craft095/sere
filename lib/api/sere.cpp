#include "api/sere.hpp"

#include "ast/Parser.hpp"
#include "ast/SereExpr.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "nfasl/Dfasl.hpp"
#include "rt/RtDfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

#include <cstdint>
#include <vector>

/**
 * Opaque (for clients) structure to
 * keep reference to service objects
 */

struct sere_ref {
  std::vector<uint8_t> rt;
  std::vector<std::string> atomics;
};

struct sere_context {
  rt::ExecutorPtr context;
};

/**
 * Release SERE compilation results
 */
void sere_release(sere_compiled* result) {
  if (result->compiled) {
    delete result->ref;
    delete [] result->atomics;
  }
}

/**
 * Compile SERE expression
 *
 * @param[in] expr NUL terminated SERE expression
 * @param[in] opts options to control compilation
 * @param[out] result compilation results
 */
int sere_compile(const char* expr,
                 const struct sere_options* opts,
                 sere_compiled* result) {
  try {
    std::istringstream stream(expr);
    parser::ParseResult r = parser::parse(stream);

    Ptr<SereExpr> expr = r.expr;
    const std::map<std::string, size_t>& vars = r.vars;

    nfasl::Nfasl nfa = sereToNfasl(*expr);
    nfasl::Nfasl min;
    nfasl::minimize(nfa, min);

    dfasl::Dfasl dfa;
    dfasl::toDfasl(min, dfa);

    rt::Dfasl rtDfa;
    dfasl::toRt(dfa, rtDfa);

    result->compiled = 1;
    result->ref = new sere_ref;

    result->atomics = new const char* [nfa.atomicCount];
    result->atomics_count = nfa.atomicCount;
    result->ref->atomics.resize(nfa.atomicCount);
    for (auto v : vars) {
      result->ref->atomics[v.second] = v.first;
      result->atomics[v.second] = result->ref->atomics[v.second].c_str();
    }

    write(rtDfa, result->ref->rt);

    result->rt = &(result->ref->rt)[0];
    result->rt_size = result->ref->rt.size();
    return 0;
  } catch(std::invalid_argument& ex) {
    result->compiled = 0;
    return -1;
  }
}

int sere_context_load(const uint8_t* rt, /** serialized *FASL */
                      size_t rt_size, /** serialized *FASL size */
                      void** sere /** loaded SERE */
                      ) {
  struct sere_context* ctx = nullptr;
  try {
    rt::ExecutorPtr context = rt::Loader::load(rt, rt_size);
    ctx = new sere_context;
    ctx->context = context;
    *sere = reinterpret_cast<void*>(ctx);
    return 0;
  } catch(rt::LoadingFailed& ex) {
    return -1;
  }
}

void sere_context_release(void* ctx) {
  delete reinterpret_cast<sere_context*>(ctx);
}

void sere_context_reset(void* ctx) {
  reinterpret_cast<sere_context*>(ctx)->context->reset();
}

void sere_context_advance(void* ctx,
                          const char* atomics,
                          size_t atomics_count) {
  rt::Names vars;
  for (size_t ix = 0; ix < atomics_count; ++ix) {
    vars.set(ix, atomics[ix]);
  }
  reinterpret_cast<sere_context*>(ctx)->context->advance(vars);
}

void sere_context_get_result(void* ctx, int* result) {
  *result = reinterpret_cast<sere_context*>(ctx)->context->getResult();
}

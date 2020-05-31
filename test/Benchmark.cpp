#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"

#include <benchmark/benchmark.h>

#include <string>

static void generate(size_t word_count, std::string& nm) {
  constexpr size_t word_size = 5;

  // deterministic expression
  std::srand(42);

  auto genLetter = []() {
                     const char letters[] = "abcdefghijklmnopqrstuvwxyz";
                     constexpr size_t letter_size = sizeof(letters) - 1;
                     return letters[std::rand() % letter_size];
                   };

  nm = "false";
  for (size_t w = 0; w < word_count; ++w) {
    std::string word;
    word.push_back(genLetter());

    for (size_t l = 1; l < word_size; ++l) {
      word.push_back(';');
      word.push_back(genLetter());
    }
    nm += "| (";
    nm += word;
    nm += ")";
  }
}


static Ptr<nfasl::Nfasl> nfaslParse(const char* expr) {
  std::istringstream stream(expr);
  parser::ParseResult r = parser::parse("<buffer>", stream);

  Ptr<nfasl::Nfasl> nfasl = std::make_shared<nfasl::Nfasl>(sereToNfasl(*r.expr));
  return nfasl;
}

static void nfaslMinimization(const nfasl::Nfasl& nfa, size_t& minSt) {
  nfasl::Nfasl min;
  nfasl::minimize(nfa, min);
  minSt = min.stateCount;
}

static void BM_NfaslMinimization(benchmark::State& state) {
  size_t minSt = 0;
  std::string expr;
  generate(state.range(0), expr);
  Ptr<nfasl::Nfasl> e0 = nfaslParse(expr.c_str());
  for (auto _ : state)
    nfaslMinimization(*e0, minSt);
  state.counters["states"] = e0->stateCount;
  state.counters["min-states"] = minSt;
}

BENCHMARK(BM_NfaslMinimization)->Arg(10)->Arg(20)->Arg(30)->Arg(40)->Arg(50;

BENCHMARK_MAIN();

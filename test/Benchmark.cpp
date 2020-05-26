#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"

#include <benchmark/benchmark.h>

#include <string>

const char mediumExpr[] =
  "("
  "(A || B || C)[+] | ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((Y ; X ; F) ; D)"
  ":"
  "(L || M || N)[+] | ((H && B && O) ; D)"
  ":"
  "(U ; B ; V)[+] & ((Z || X && W) ; D)"
  ":"
  "(U || B || V)[+] ; ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((Y ; X ; F) ; D)"
  ":"
  "(L || M || N)[+] | ((H && B && O) ; D)"
  ":"
  "(U ; B ; V)[+] & ((Z || X && W) ; D)"
  "){10}"
  "&"
  "("
  "(A || B || C)[+] | ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((Y ; X ; F) ; D)"
  ":"
  "(L || M || N)[+] | ((H && B && O) ; D)"
  ":"
  "(U ; B ; V)[+] & ((Z || X && W) ; D)"
  ":"
  "(U || B || V)[+] ; ((C && E && F) ; D)"
  ":"
  "(U || B || V)[+] ; ((Y ; X ; F) ; D)"
  ":"
  "(L || M || N)[+] | ((H && B && O) ; D)"
  ":"
  "(U ; B ; V)[+] & ((Z || X && W) ; D)"
  "){10}"
  ;

static void generate(std::string& nm) {
  constexpr size_t word_size = 5;
  constexpr size_t word_count = 300;

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

  //Ptr<SereExpr> expr = r.expr;
  //const std::map<std::string, size_t>& vars = r.vars;
  Ptr<nfasl::Nfasl> nfasl = std::make_shared<nfasl::Nfasl>(sereToNfasl(*r.expr));
  return nfasl;
}

static void nfaslMinimization(const nfasl::Nfasl& nfa, size_t& minSt) {
  nfasl::Nfasl min;
  nfasl::minimize(nfa, min);
  minSt = min.stateCount;
}

static void BM_MediumNfaslMinimization(benchmark::State& state) {
  size_t minSt = 0;
  std::string expr;
  generate(expr);
  Ptr<nfasl::Nfasl> e0 = nfaslParse(expr.c_str());
  for (auto _ : state)
    nfaslMinimization(*e0, minSt);
  state.counters["states"] = e0->stateCount;
  state.counters["min-states"] = minSt;
}
// static void BM_MediumNfaslMinimization(benchmark::State& state) {
//   size_t minSt = 0;
//   Ptr<nfasl::Nfasl> e0 = nfaslParse(mediumExpr);
//   for (auto _ : state)
//     nfaslMinimization(*e0, minSt);
//   state.counters["states"] = e0->stateCount;
//   state.counters["min-states"] = minSt;
// }
// Register the function as a benchmark
BENCHMARK(BM_MediumNfaslMinimization);

// static void BM_BigNfaslMinimization(benchmark::State& state) {
//   Ptr<SereExpr> e0 = nfaslParse(bigExpr);
//   for (auto _ : state)
//     nfaslMinimization(*e0);
// }
// // Register the function as a benchmark
// BENCHMARK(BM_BigNfaslMinimization);

BENCHMARK_MAIN();

#include "Match.hpp"
#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"

#include "test/Tools.hpp"
#include "test/Letter.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalRt.hpp"
#include "test/EvalNfasl.hpp"

#include <sstream>
#include "catch2/catch.hpp"

Word toWord(const parser::AtomicNameMap& map,
            const std::vector<std::string>& stream) {
  Word word;

  for (auto const& l : stream) {
    rt::Names names;
    names.resize(map.size());
    names.set(map.at(l));
    word.push_back(names);
  }

  return word;
}

static
void prepareExprs(const char* u, nfasl::Nfasl& expr1, parser::AtomicNameMap& map) {
  std::istringstream stream0(u);
  parser::ParseResult r0 = parser::parse(stream0);
  map = r0.vars;
  nfasl::Nfasl expr0 = sereToNfasl(*r0.expr);
  nfasl::clean(expr0, expr1);
}

TEST_CASE("Nfasl Extended") {
  nfasl::Nfasl nfasl;
  rt::Nfasl rtNfasl;
  parser::AtomicNameMap map;

  const char* expr0 = "(A;B) | (A;B;B) | (B;B)[+] | (B;B;B;B) | (C & false)";

  prepareExprs(expr0, nfasl, map);
  toRt(nfasl, rtNfasl);

  std::vector<std::string> stream0 = {"A","B","B","B","C"};

  for (size_t sz = 0; sz <= stream0.size(); ++sz) {
    std::vector<std::string> stream(stream0.begin(), stream0.begin() + sz);

    auto word = toWord(map, stream);
    ExtendedMatch match = evalExtendedNfasl(nfasl, word);
    ExtendedMatch match1 = evalExtendedRtNfasl(rtNfasl, word);

    CHECK(match == match1);

    switch (sz) {
    case 0:
      CHECK(match.match == Match_Partial);
      CHECK(match.partial.horizon == 0);
      break;
    case 1:
      CHECK(match.match == Match_Partial);
      CHECK(match.partial.horizon == 1);
      break;
    case 2:
      CHECK(match.match == Match_Ok);
      CHECK(match.ok.shortest == 2);
      CHECK(match.ok.longest == 2);
      CHECK(match.ok.horizon == 2);
      break;
    case 3:
      CHECK(match.match == Match_Ok);
      CHECK(match.ok.shortest == 2);
      CHECK(match.ok.longest == 3);
      CHECK(match.ok.horizon == 3);
      break;
    case 4:
      CHECK(match.match == Match_Ok);
      CHECK(match.ok.shortest == 2);
      CHECK(match.ok.longest == 2);
      CHECK(match.ok.horizon == 3);
      break;
    case 5:
      CHECK(match.match == Match_Partial);
      CHECK(match.partial.horizon == 0);
      break;
    }
  }
}

#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Z3.hpp"
#include "Nfasl.hpp"
#include "TestTools.hpp"
#include "TestBoolExpr.hpp"

using json = nlohmann::json;

namespace nfasl {
  static State makeState(size_t states) {
    return choose((State)0, states - 1);
  }

  static States makeStates(size_t mn, size_t mx, size_t states) {
    auto f = [states]() { return makeState(states); };
    return set_of<State>(mn, mx, f);
  }

  static TransitionRule makeRule(size_t depth, size_t atoms, size_t states) {
    return { makeZex(depth, atoms), makeState(states) };
  }

  static TransitionRules
  makeTransitionRules(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
    auto g = [depth, atoms, states]() { return makeRule(depth, atoms, states); };
    return vector_of<TransitionRule>(0, maxTrs, g);
  }

  Nfasl
  makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
    Nfasl a;
    a.atomicCount = atoms;
    a.stateCount = states;
    a.initial = makeState(states);
    a.finals = makeStates(0, states, states);
    a.transitions.reserve(states);
    for (size_t s = 0; s < states; ++s) {
      a.transitions.push_back(makeTransitionRules(depth, atoms, states, maxTrs));
    }
    return a;
  }

  static void to_json(json& j, const TransitionRule& p) {
    j = json{{"phi", p.phi}, {"state", p.state}};
  }

  void to_json(json& j, const Nfasl& a) {
    j = json{
      {"atomicCount", a.atomicCount},
      {"stateCount",  a.stateCount},
      {"initial",     a.initial},
      {"finals",      a.finals},
      {"transitions", a.transitions}
    };
  }

  std::string pretty(const Nfasl& a) {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(a).dump(spaces) << std::endl;
    return s.str();
  }

} // namespace nfasl

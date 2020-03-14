#ifndef NFASL_HPP
#define NFASL_HPP

#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Z3.hpp"
#include "TestTools.hpp"
#include "TestBoolExpr.hpp"

using json = nlohmann::json;

namespace nfasl {
  typedef z3::expr Predicate;
  typedef size_t State;
  typedef std::set<State> States;

  struct TransitionRule {
    Predicate phi;
    State state;
  };

  typedef std::vector<TransitionRule> TransitionRules;

  class Nfasl {
  public:
    size_t atomicCount;
    size_t stateCount;
    State initial;
    States finals;
    std::vector<TransitionRules> transitions;
  };

  extern Nfasl makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs);
  extern void to_json(json& j, const Nfasl& a);
  extern std::string pretty(const Nfasl& a);
} // namespace nfasl

#endif // NFASL_HPP

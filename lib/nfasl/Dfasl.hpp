#ifndef DFASL_HPP
#define DFASL_HPP

#include <set>
#include <vector>
#include <nlohmann/json.hpp>
#include "boolean/Expr.hpp"

using json = nlohmann::json;

namespace rt {
  class Dfasl;
}

namespace nfasl {
  class Nfasl;
}

namespace dfasl {
  typedef boolean::Expr Predicate;
  typedef size_t State;
  typedef std::set<State> States;

  struct TransitionRule {
    Predicate phi;
    State state;
  };

  typedef std::vector<TransitionRule> TransitionRules;

  class Dfasl {
  public:
    size_t atomicCount;
    size_t stateCount;
    State initial;
    States finals;
    std::vector<TransitionRules> transitions;
  };

  extern void toDfasl(const nfasl::Nfasl& a, Dfasl& b);
  extern void to_json(json& j, const Dfasl& a);
  extern std::string pretty(const Dfasl& a);
  // extern void toRt(const Dfasl& u, rt::Dfasl& v);

} // namespace dfasl

#endif // DFASL_HPP

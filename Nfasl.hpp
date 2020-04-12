#ifndef NFASL_HPP
#define NFASL_HPP

#include <set>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Z3.hpp"
#include "Language.hpp"

using json = nlohmann::json;

namespace rt {
  class Nfasl;
}

namespace nfasl {
  typedef VarName AtomicName;
  typedef Ptr<BoolExpr> Predicate;
  typedef size_t State;
  typedef std::set<State> States;

  struct TransitionRule {
    Predicate phi;
    State state;
  };

  typedef std::vector<TransitionRule> TransitionRules;

  class Nfasl {
  public:
    std::set<AtomicName> atomics;
    size_t atomicCount;
    size_t stateCount;
    State initial;
    States finals;
    std::vector<TransitionRules> transitions;
  };

  extern Nfasl eps();
  extern Nfasl phi(Ptr<BoolExpr> expr);
  extern Nfasl unions(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl intersects(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl concat(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl fuse(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl kleeneStar(const Nfasl& a0);
  extern Nfasl kleenePlus(const Nfasl& a0);
  extern Nfasl partial(const Nfasl& a0);

  extern void to_json(json& j, const Nfasl& a);
  extern std::string pretty(const Nfasl& a);
  extern void toRt(const Nfasl& u, rt::Nfasl& v);

} // namespace nfasl

#endif // NFASL_HPP

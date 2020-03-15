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
#include "TestTools.hpp"
#include "TestBoolExpr.hpp"

using json = nlohmann::json;

namespace nfasl {
  typedef VarName AtomicName;
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
    std::set<AtomicName> atomics;
    size_t atomicCount;
    size_t stateCount;
    State initial;
    States finals;
    std::vector<TransitionRules> transitions;
  };

  extern Nfasl eps();
  extern Nfasl phi(BoolExpr& expr);
  extern Nfasl unions(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl intersects(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl concat(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl fuse(const Nfasl& a0, const Nfasl& a1);
  extern Nfasl kleeneStar(const Nfasl& a0);
  extern Nfasl kleenePlus(const Nfasl& a0);

  extern Ptr<Nfasl> makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs);
  extern void to_json(json& j, const Nfasl& a);
  extern std::string pretty(const Nfasl& a);
} // namespace nfasl

#endif // NFASL_HPP

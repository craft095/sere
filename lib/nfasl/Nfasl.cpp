#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "sat/Z3.hpp"
#include "rt/RtPredicate.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Algo.hpp"

using json = nlohmann::json;

namespace nfasl {
  static void from_json(const json& j, TransitionRule& p) {
    j.at("phi").get_to(p.phi);
    j.at("state").get_to(p.state);
  }

  static void to_json(json& j, const TransitionRule& p) {
    j = json{{"phi", p.phi}, {"state", p.state}};
  }

  void from_json(const json& j, Nfasl& a) {
    j.at("atomicCount").get_to(a.atomicCount);
    j.at("stateCount").get_to(a.stateCount);
    j.at("initial").get_to(a.initial);
    j.at("finals").get_to(a.finals);
    j.at("transitions").get_to(a.transitions);
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

  Nfasl eps() {
    Nfasl a;
    a.atomicCount = 0;
    a.stateCount = 1;
    a.initial = 0;
    a.finals.insert(a.initial);
    a.transitions.resize(a.stateCount);
    return a;
  }

  Nfasl phi(Predicate expr) {
    Nfasl a;
    constexpr State ini = 0;
    constexpr State fin = 1;

    a.atomicCount = expr.var_count();
    a.stateCount = 2;
    a.initial = ini;
    a.finals.insert(fin);
    a.transitions.resize(a.stateCount);
    a.transitions[ini].push_back({ expr, fin });

    return a;
  }

  Nfasl intersects(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomicCount = std::max(a0.atomicCount, a1.atomicCount);
    a.stateCount = a0.stateCount * a1.stateCount;

    std::function<State(State,State)> remap = [a1](State s0, State s1) { return s0*a1.stateCount + s1; };

    a.initial = remap(a0.initial, a1.initial);
    a.finals = set_cross_with(a0.finals, a1.finals, remap);
    a.transitions.resize(a.stateCount);
    for (State s0 = 0; s0 < a0.stateCount; ++s0) {
      for (State s1 = 0; s1 < a1.stateCount; ++s1) {
        for (auto const& rule0 : a0.transitions[s0]) {
          for (auto const& rule1 : a1.transitions[s1]) {
            TransitionRule rule = { rule0.phi && rule1.phi,
                                    remap(rule0.state, rule1.state) };
            a.transitions[remap(s0,s1)].push_back(rule);
          }
        }
      }
    }
    return a;
  }

  Nfasl unions(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomicCount = std::max(a0.atomicCount, a1.atomicCount);
    a.stateCount = a0.stateCount + a1.stateCount + 1;

    auto remap0 = [](State s0) { return s0; };
    auto remap1 = [a0](State s1) { return a0.stateCount + s1; };

    a.initial = a.stateCount - 1;

    for (auto s0 : a0.finals) {
      a.finals.insert(remap0(s0));
    }
    for (auto s1 : a1.finals) {
      a.finals.insert(remap1(s1));
    }
    if (set_member(a0.finals, a0.initial) || set_member(a1.finals, a1.initial)) {
      a.finals.insert(a.initial);
    }

    a.transitions.resize(a.stateCount);

    for (State s0 = 0; s0 < a0.stateCount; ++s0) {
      for (auto const& rule : a0.transitions[s0]) {
        a.transitions[remap0(s0)].push_back({ rule.phi, remap0(rule.state) });
      }
    }
    for (State s1 = 0; s1 < a1.stateCount; ++s1) {
      for (auto const& rule : a1.transitions[s1]) {
        a.transitions[remap1(s1)].push_back({ rule.phi, remap1(rule.state) });
      }
    }

    for (auto const& rule : a0.transitions[a0.initial]) {
      a.transitions[a.initial].push_back({ rule.phi, remap0(rule.state) });
    }
    for (auto const& rule : a1.transitions[a1.initial]) {
      a.transitions[a.initial].push_back({ rule.phi, remap1(rule.state) });
    }
    return a;
  }

  Nfasl concat(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomicCount = std::max(a0.atomicCount, a1.atomicCount);
    a.stateCount = a0.stateCount + a1.stateCount;

    auto remap0 = [](State s0) { return s0; };
    auto remap1 = [a0](State s1) { return a0.stateCount + s1; };

    a.initial = remap0(a0.initial);

    if (set_member(a1.finals, a1.initial)) {
      for (auto s0 : a0.finals) {
        a.finals.insert(remap0(s0));
      }
    }
    for (auto s1 : a1.finals) {
      a.finals.insert(remap1(s1));
    }

    a.transitions.resize(a.stateCount);

    for (State s0 = 0; s0 < a0.stateCount; ++s0) {
      for (auto const& rule : a0.transitions[s0]) {
        a.transitions[remap0(s0)].push_back({ rule.phi, remap0(rule.state) });
      }
      if (set_member(a0.finals, s0)) {
        for (auto const& rule : a1.transitions[a1.initial]) {
          a.transitions[remap0(s0)].push_back({ rule.phi, remap1(rule.state) });
        }
      }
    }

    for (State s1 = 0; s1 < a1.stateCount; ++s1) {
      for (auto const& rule : a1.transitions[s1]) {
        a.transitions[remap1(s1)].push_back({ rule.phi, remap1(rule.state) });
      }
    }

    return a;
  }

  Nfasl fuse(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomicCount = std::max(a0.atomicCount, a1.atomicCount);
    a.stateCount = a0.stateCount + a1.stateCount;

    auto remap0 = [](State s0) { return s0; };
    auto remap1 = [a0](State s1) { return a0.stateCount + s1; };

    a.initial = remap0(a0.initial);

    for (auto s1 : a1.finals) {
      a.finals.insert(remap1(s1));
    }

    a.transitions.resize(a.stateCount);

    for (State s0 = 0; s0 < a0.stateCount; ++s0) {
      for (auto const& rule : a0.transitions[s0]) {
        a.transitions[remap0(s0)].push_back({ rule.phi, remap0(rule.state) });

        // if we step into final from this state
        // then we must join next-after-initial states
        // from a1
        if (set_member(a0.finals, rule.state)) {
          for (auto const& rule1 : a1.transitions[a1.initial]) {
            a.transitions[remap0(s0)].push_back({ rule.phi && rule1.phi,
                                                  remap1(rule1.state) });
          }

        }
      }
    }

    for (State s1 = 0; s1 < a1.stateCount; ++s1) {
      for (auto const& rule : a1.transitions[s1]) {
        a.transitions[remap1(s1)].push_back({ rule.phi, remap1(rule.state) });
      }
    }

    return a;
  }

  Nfasl kleeneStar(const Nfasl& a0) {
    Nfasl a;
    a.atomicCount = a0.atomicCount;
    a.stateCount = a0.stateCount;

    a.initial = a0.initial;
    a.finals = a0.finals;
    a.finals.insert(a.initial);

    a.transitions = a0.transitions;

    auto const& initialTransitions = a0.transitions[a0.initial];
    for (State s = 0; s < a.stateCount; ++s) {
      auto&  transitions = a.transitions[s];
      if (set_member(a0.finals, s)) {
        transitions.insert(transitions.end(),
                           initialTransitions.begin(), initialTransitions.end());
      }
    }

    return a;
  }

  Nfasl kleenePlus(const Nfasl& a0) {
    Nfasl a;
    a.atomicCount = a0.atomicCount;
    a.stateCount = a0.stateCount;

    a.initial = a0.initial;
    a.finals = a0.finals;

    a.transitions = a0.transitions;

    auto const& initialTransitions = a0.transitions[a0.initial];
    for (State s = 0; s < a.stateCount; ++s) {
      auto&  transitions = a.transitions[s];
      if (set_member(a0.finals, s)) {
        transitions.insert(transitions.end(),
                           initialTransitions.begin(), initialTransitions.end());
      }
    }

    return a;
  }

  /**
   * `Partial` makes every state as final,
   * But "every" means only reachable from initial
   * _and_ from which finals are reachable.
   * So it is easier to clean automaton first.
   */
  Nfasl partial(const Nfasl& a0) {
    Nfasl a;
    clean(a0, a);

    if (a.finals.empty()) {
      return a;
    }

    for (State q = 0; q < a.stateCount; ++q) {
      a.finals.insert(q);
    }
    return a;
  }

  void toRt(const Nfasl& u, rt::Nfasl& v) {
    v.atomicCount = u.atomicCount;
    v.stateCount = u.stateCount;
    v.initials.resize(v.stateCount);
    v.initials.set(u.initial);
    v.finals.resize(v.stateCount);
    for (auto q : u.finals) {
      v.finals.set(q);
    }
    v.transitions.resize(v.stateCount);
    for (State q = 0; q < u.stateCount; ++q) {
      TransitionRules uT = u.transitions[q];
      v.transitions[q].resize(uT.size());
      auto vRule = v.transitions[q].begin();
      for (auto& rule : uT) {
        boolean::toRtPredicate(rule.phi, vRule->phi);
        vRule->state = rule.state;
        ++vRule;
      }
    }
  }

} // namespace nfasl

#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Z3.hpp"
#include "Nfasl.hpp"
#include "TestTools.hpp"
#include "TestBoolExpr.hpp"
#include "Algo.hpp"

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
    for (size_t ix = 0; ix < atoms; ++ix) {
      a.atomics.insert(std::string(char('a' + ix), 1));
    }
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
      {"atomics",     a.atomics},
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

  Nfasl intersect(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomics = set_unions(a0.atomics, a1.atomics);
    a.atomicCount = a.atomics.size();
    a.stateCount = a0.stateCount * a1.stateCount;

    std::function<State(State,State)> remap = [a1](State s0, State s1) { return s0*a1.stateCount + s1; };

    a.initial = remap(a0.initial, a1.initial);
    a.finals = set_cross_with(a0.finals, a1.finals, remap);
    a.transitions.resize(a.stateCount);
    for (State s0 = 0; s0 < a0.stateCount; ++s0) {
      for (State s1 = 0; s1 < a1.stateCount; ++s1) {
        for (auto const& rule0 : a0.transitions[s0]) {
          for (auto const& rule1 : a1.transitions[s1]) {
            TransitionRule rule = { rule0.phi && rule1.phi, remap(s0, s1) };
            a.transitions[remap(s0,s1)].push_back(rule);
          }
        }
      }
    }
    return a;
  }

  Nfasl unions(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomics = set_unions(a0.atomics, a1.atomics);
    a.atomicCount = a.atomics.size();
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

} // namespace nfasl

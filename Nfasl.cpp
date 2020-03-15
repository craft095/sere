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

  Ptr<Nfasl>
  makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
    Ptr<Nfasl> a = std::make_shared<Nfasl>();
    for (size_t ix = 0; ix < atoms; ++ix) {
      a->atomics.insert(make_varName(ix));
    }
    a->atomicCount = atoms;
    a->stateCount = states;
    a->initial = makeState(states);
    a->finals = makeStates(0, states, states);
    a->transitions.reserve(states);
    for (size_t s = 0; s < states; ++s) {
      a->transitions.push_back(makeTransitionRules(depth, atoms, states, maxTrs));
    }
    return a;
  }

  static void to_json(json& j, const VarName& var) {
    j = json{{"var", var.ix}};
  }

  static void to_json(json& j, const TransitionRule& p) {
    j = json{{"phi", p.phi}, {"state", p.state}};
  }

  void to_json(json& j, const Nfasl& a) {
    j = json{
      // {"atomics",     a.atomics},
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

  Nfasl phi(BoolExpr& expr) {
    Nfasl a;
    constexpr State ini = 0;
    constexpr State fin = 1;

    std::set<VarName> vars = boolExprGetAtomics(expr);
    size_t atomicCount = 0;
    for (auto const& var : vars) {
      atomicCount = std::max(atomicCount, var.ix + 1);
    }
    for (size_t i = 0; i < atomicCount; ++i) {
      a.atomics.insert(make_varName(i));
    }

    a.atomicCount = a.atomics.size();
    a.stateCount = 2;
    a.initial = ini;
    a.finals.insert(fin);
    a.transitions.resize(a.stateCount);
    a.transitions[ini].push_back({ boolSereToZex(expr), fin });

    return a;
  }

  Nfasl intersects(const Nfasl& a0, const Nfasl& a1) {
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
            TransitionRule rule = { rule0.phi && rule1.phi, remap(rule0.state, rule1.state) };
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

  Nfasl concat(const Nfasl& a0, const Nfasl& a1) {
    Nfasl a;
    a.atomics = set_unions(a0.atomics, a1.atomics);
    a.atomicCount = a.atomics.size();
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
    a.atomics = set_unions(a0.atomics, a1.atomics);
    a.atomicCount = a.atomics.size();
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
            a.transitions[remap0(s0)].push_back({ rule.phi && rule1.phi, remap1(rule1.state) });
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
    a.atomics = a0.atomics;
    a.atomicCount = a.atomics.size();
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
    a.atomics = a0.atomics;
    a.atomicCount = a.atomics.size();
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

} // namespace nfasl

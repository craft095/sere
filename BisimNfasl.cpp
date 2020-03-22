#include <set>
#include <map>
#include <optional>

#include "Algo.hpp"
#include "DAG.hpp"
#include "Nfasl.hpp"
#include "BisimNfasl.hpp"

/**
 * Minimize NFASL state count with bisimulation.
 * See http://pages.cs.wisc.edu/~loris/papers/tacas17bisimulations.pdf
 */

namespace nfasl {
  void filterStates(const Nfasl& nfasl,
                    const std::set<State> states,
                    Nfasl& cleaned) {
    std::map<State, State> remap;
    size_t st_new = 0;
    for (auto q : states) {
      remap[q] = st_new++;
    }

    auto remapF = [&remap](State s) { return get(remap, s); };

    cleaned.atomics = nfasl.atomics;
    cleaned.atomicCount = states.size();
    cleaned.initial = *remapF(nfasl.initial);
    for (auto q : nfasl.finals) {
      auto v = remapF(q);
      if (v) {
        cleaned.finals.insert(*v);
      }
    }

    cleaned.transitions.resize(cleaned.stateCount);
    for (State q = 0; q < nfasl.stateCount; ++q) {
      auto new_q = remapF(q);
      if (new_q) {
        TransitionRules& new_rules = cleaned.transitions[*new_q];
        const TransitionRules& rules = nfasl.transitions[q];

        for (auto rule : rules) {
          auto new_tgt = remapF(rule.state);
          if (new_tgt) {
            new_rules.push_back(TransitionRule { rule.phi, *new_tgt });
          }
        }
      }
    }
  }

  /**
   * Automaton is normalized if there is at most one   * transition rule between any two states
   *
   * For any two states, the algo joins all rules with OR into single rule
   */
  // void normalize(const Nfasl& nfasl) {
  // }

  void clean(const Nfasl& nfasl, Nfasl& cleaned) {
    DAG<State> forward, backward;

    for (State s = 0; s < nfasl.stateCount; ++s) {
      for (auto const& rule : nfasl.transitions[s]) {
        if (satisfiable(rule.phi)) {
          forward.arcs[s].insert(rule.state);
          backward.arcs[rule.state].insert(s);
        }
      }
    }

    std::set<State> initials { nfasl.initial };
    std::set<State> reachableFromInitial;
    forward.reachableFrom(initials, reachableFromInitial);
    std::set<State> reachableFromFinal;
    backward.reachableFrom(nfasl.finals, reachableFromFinal);

    std::set<State> reachable;
    reachable = set_intersects(reachableFromInitial, reachableFromFinal);

    if (reachable.empty()) {
      reachable.insert(nfasl.initial);
    }

    filterStates(nfasl, reachable, cleaned);

    // remove infeasible rules
    for (auto& trs : cleaned.transitions) {
      TransitionRules filtered_rules;
      for (auto& rule : trs) {
        if (satisfiable(rule.phi)) {
          filtered_rules.push_back(rule);
        }
      }
      std::swap(trs, filtered_rules);
    }
  }

  // Nfasl minimize() {
  // }

}

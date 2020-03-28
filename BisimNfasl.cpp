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
    cleaned.atomicCount = nfasl.atomicCount;
    cleaned.stateCount = states.size();
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
        if (satisfiable(boolSereToZex(*rule.phi))) {
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
        if (satisfiable(boolSereToZex(*rule.phi))) {
          filtered_rules.push_back(rule);
        }
      }
      std::swap(trs, filtered_rules);
    }
  }

  static void joinStates(const Nfasl& a, const std::set<States>& partition, Nfasl& b) {
    b.atomics = a.atomics;
    b.atomicCount = a.atomicCount;
    b.stateCount = partition.size();

    std::map<State, State> remap;

    State newQ = 0;
    for (auto const& qs : partition) {
      for (auto q : qs) {
        remap[q] = newQ;
      }
      newQ += 1;
    }

    assert(newQ == b.stateCount);

    auto remapF = [&remap](State q) { return remap[q]; };

    b.initial = remapF(a.initial);
    b.transitions.resize(b.stateCount);
    for (State q = 0; q < a.stateCount; ++q) {
      for (auto const& rule : a.transitions[q]) {
        b.transitions[q].push_back({ rule.phi, remapF(rule.state) });
      }
    }

    for (auto q : a.finals) {
      b.finals.insert(remapF(q));
    }
  }

  static Ptr<BoolExpr> delta(const Nfasl& a, State q, State r) {
    assert(q < a.stateCount);
    assert(r < a.stateCount);

    Ptr<BoolExpr> ret = RE_FALSE;
    for (auto const& rule : a.transitions[q]) {
      if (rule.state == r) {
        ret = RE_OR(ret, rule.phi);
      }
    }

    return ret;
  }

  static Ptr<BoolExpr> delta(const Nfasl& a, State q, States R) {
    Ptr<BoolExpr> ret = RE_FALSE;
    for (auto const& r : R) {
      ret = RE_OR(ret, delta(a, q, r));
    }

    return ret;
  }

  static void simpleBisim(const Nfasl& a, std::set<States>& partition) {
    std::set<States> P, W;
    States Q;

    for (State q = 0; q < a.stateCount; ++q) {
      Q.insert(q);
    }

    P.insert(a.finals);
    P.insert(set_difference(Q, a.finals));

    W = P;

    while (!W.empty()) {
      States R;
      R = *W.begin();
      W.erase(W.begin());

      for (auto const& B : P) {
        State q, r;
        bool found = false;
        Ptr<BoolExpr> DqNotDr;

        for (auto qx : B) {
          for (auto rx : B) {
            if (qx != rx) {
              q = qx;
              r = rx;

              DqNotDr = RE_AND(delta(a, q, R), RE_NOT(delta(a, r, R)));

              if (satisfiable(boolSereToZex(*DqNotDr))) {
                found = true;
                break;
              }
            }
          }
          if (found) { break; }
        }
        if (found) {
          States D, B_dif_D;

          for (auto p : B) {
            if (satisfiable(boolSereToZex(*RE_AND(delta(a, p, R), DqNotDr)))) {
              D.insert(p);
            } else {
              B_dif_D.insert(p);
            }
          }
          P.erase(B);
          P.insert(D);
          P.insert(B_dif_D);
          W.erase(B);
          W.insert(D);
          W.insert(B_dif_D);
          break;
        }
      }
    }

    std::swap(P, partition);
  }

  void minimize(const Nfasl& a, Nfasl& b) {
    Nfasl c;
    clean(a, c);

    std::set<States> partition;
    simpleBisim(c, partition);

    Nfasl d;
    joinStates(c, partition, d);

    clean(d, b);
  }

}

#include <set>
#include <map>
#include <optional>

#include "Algo.hpp"
#include "sat/Sat.hpp"
#include "sat/Simplify.hpp"
#include "nfasl/DAG.hpp"
#include "nfasl/Dfasl.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"

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
        if (sat(rule.phi)) {
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
        if (sat(rule.phi)) {
          filtered_rules.push_back(rule);
        }
      }
      std::swap(trs, filtered_rules);
    }
  }

  static void joinStates(const Nfasl& a, const std::set<States>& partition, Nfasl& b) {
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
        b.transitions[remapF(q)].push_back({ rule.phi, remapF(rule.state) });
      }
    }

    for (auto q : a.finals) {
      b.finals.insert(remapF(q));
    }
  }

  static Predicate delta(const Nfasl& a, State q, State r) {
    assert(q < a.stateCount);
    assert(r < a.stateCount);

    Predicate ret = Predicate::value(false);
    for (auto const& rule : a.transitions[q]) {
      if (rule.state == r) {
        ret = ret || rule.phi;
      }
    }

    return ret;
  }

  static Predicate delta(const Nfasl& a, State q, const States& R) {
    Predicate ret = Predicate::value(false);
    for (auto r : R) {
      ret = ret || delta(a, q, r);
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
        Predicate DqNotDr;

        for (auto qx : B) {
          for (auto rx : B) {
            if (qx != rx) {
              q = qx;
              r = rx;

              DqNotDr = delta(a, q, R) && !delta(a, r, R);

              if (sat(DqNotDr)) {
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
            if (sat(delta(a, p, R) && DqNotDr)) {
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
        }
      }
    }

    std::swap(P, partition);
  }

  static void greedyBisim(const Nfasl& a, std::set<States>& partition) {
    std::set<States> P, W;
    States Q;

    for (State q = 0; q < a.stateCount; ++q) {
      Q.insert(q);
    }

    States QsubF(set_difference(Q, a.finals));

    P.insert(a.finals);
    P.insert(QsubF);

    if (a.finals.size() <= QsubF.size()) {
      W.insert(a.finals);
    } else {
      W.insert(QsubF);
    }

    std::map<States, States> SUPER;
    SUPER[a.finals] = Q;
    SUPER[QsubF] = Q;

    while (!W.empty()) {
      States R, R_prime;
      R = *W.begin();
      W.erase(W.begin());

      R_prime = set_difference(SUPER[R], R);

      bool restart_iteration_over_P = true;
      while (restart_iteration_over_P) {
        restart_iteration_over_P = false;
        for (auto const& B : P) {
          State q, r;
          bool found_in_R = false;
          bool found_in_R_prime = false;
          Predicate DqNotDr;
          Predicate DqNotDr_prime;

          for (auto qx : B) {
            for (auto rx : B) {
              if (qx != rx) {
                q = qx;
                r = rx;

                DqNotDr = delta(a, q, R) && !delta(a, r, R);

                if (sat(DqNotDr)) {
                  found_in_R = true;
                  break;
                }

                DqNotDr_prime = delta(a, q, R_prime) && !delta(a, r, R_prime);

                if (sat(DqNotDr_prime)) {
                  found_in_R_prime = true;
                  break;
                }
              }
            }
            if (found_in_R || found_in_R_prime) { break; }
          }
          if (found_in_R || found_in_R_prime) {
            States D, B_dif_D;

            if (found_in_R) {
              for (auto p : B) {
                if (sat(delta(a, p, R) && DqNotDr)) {
                  D.insert(p);
                } else {
                  B_dif_D.insert(p);
                }
              }
            } else {
              for (auto p : B) {
                if (sat(delta(a, p, R_prime) && DqNotDr_prime)) {
                  D.insert(p);
                } else {
                  B_dif_D.insert(p);
                }
              }
            }
            // refine P by splitting B
            P.erase(B);
            P.insert(D);
            P.insert(B_dif_D);

            if (set_member(W, B)) {
              W.erase(B);
              W.insert(D);
              W.insert(B_dif_D);

              auto const& SUPER_B = SUPER[B];
              SUPER[D] = SUPER_B;
              SUPER[B_dif_D] = SUPER_B;
            } else {
              if (D.size() <= B_dif_D.size()) {
                W.insert(D);
              } else {
                W.insert(B_dif_D);
              }

              SUPER[D] = B;
              SUPER[B_dif_D] = B;
            }

            //restart_iteration_over_P = true;
            break; // abort current iteration over P as P was changed
          }
        }
      }
    }
    std::swap(P, partition);
  }

  void minimize(const Nfasl& a, Nfasl& b) {
    Nfasl c;
    clean(a, c);

    std::set<States> partition;
    //simpleBisim(c, partition);
    greedyBisim(c, partition);

    joinStates(c, partition, b);
  }

  static void toNfasl(const dfasl::Dfasl& a, nfasl::Nfasl& b) {
    b.atomicCount = a.atomicCount;
    b.stateCount = a.stateCount;
    b.initial = a.initial;
    b.finals = a.finals;
    b.transitions.resize(b.stateCount);

    for (State q = 0; q < a.stateCount; ++q) {
      for (auto const& rule : a.transitions[q]) {
        b.transitions[q].push_back({rule.phi, rule.state});
      }
    }
  }

  void complement(const nfasl::Nfasl& a, nfasl::Nfasl& b) {
    dfasl::Dfasl d;
    dfasl::toDfasl(a, d);
    dfasl::complement(d);

    Nfasl u;
    toNfasl(d, u);
    clean(u, b);
  }

}

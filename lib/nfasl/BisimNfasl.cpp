#include <set>
#include <map>
#include <optional>
#include <unordered_set>

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
#if 0
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
#endif

  struct Set {
    typedef std::set<State> Payload;
    typedef std::shared_ptr<Set> Ptr;

    Ptr superBlock;
    Payload payload;
    size_t hashValue;

    Set() : superBlock(nullptr), hashValue(0) {
    }

    size_t hash() const {
      return hashValue;
    }

    size_t size() const {
      return payload.size();
    }

    void insert(State q) {
      payload.insert(q);

      // order independed hash value - plain sum works fine here
      hashValue = hashValue + q;
    }

    const States& as_set() const {
      return payload;
    }

    void setSuper(Ptr super) {
      superBlock = super;
    }

    Ptr getSuper() const {
      return superBlock;
    }

    Payload::iterator begin() {
      return payload.begin();
    }
    Payload::const_iterator begin() const {
      return payload.begin();
    }

    Payload::iterator end() {
      return payload.end();
    }
    Payload::const_iterator end() const {
      return payload.end();
    }

    static Ptr make() {
      return std::make_shared<Set>();
    }

    static Ptr make(const States& qs) {
      Ptr r {make()};
      for (auto q : qs) {
        r->insert(q);
      }
      return r;
    }
  };

  struct SetHash {
    size_t operator()(const Set::Ptr& s) const noexcept
    {
      return s->hash();
    }
  };

  struct SetEqual {
    size_t operator()(const Set::Ptr& x, const Set::Ptr& y) const noexcept
    {
      return x->payload == y->payload;
    }
  };

  typedef std::unordered_set<Set::Ptr, SetHash, SetEqual> SuperSet;

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

#ifdef CACHE_DELTA
  // pre-calculated delta functions
  // `delta(const Nfasl& a, State q, State r)`
  struct DeltaMatrix {
    const Nfasl& a;
    mutable std::vector<Predicate> table;

    size_t ix(State u, State v) const {
      return u*a.stateCount + v;
    }

    DeltaMatrix(const Nfasl& a_) : a(a_) {
      table.resize(a.stateCount*a.stateCount);
      memset(&table[0], 0, sizeof(table[0])*table.size());
    }

    Predicate get(State u, State v) const {
      Predicate& p {table[ix(u,v)]};
      if (p == boolean::Context::novalue()) {
          p = delta(a, u, v);
      }
      return p;
    }
  };
#endif

#ifndef CACHE_DELTA
  static Predicate delta(const Nfasl& a, State q, const Set& R) {
    Predicate ret = Predicate::value(false);
    for (auto r : R) {
      ret = ret || delta(a, q, r);
    }

    return ret;
  }
#else
  static Predicate delta(const DeltaMatrix m, State q, const Set& R) {
    Predicate ret = Predicate::value(false);
    for (auto r : R) {
      ret = ret || m.get(q, r);
    }

    return ret;
  }
#endif

  static void greedyBisim(const Nfasl& a, std::set<States>& partition) {
    //DeltaMatrix deltaM{a};
    SuperSet P, W;
    Set::Ptr Q{Set::make()};

    for (State q = 0; q < a.stateCount; ++q) {
      Q->insert(q);
    }

    Set::Ptr QsubF(Set::make(set_difference(Q->as_set(), a.finals)));

    Set::Ptr finals{Set::make(a.finals)};

    P.insert(finals);
    P.insert(QsubF);

    if (finals->size() <= QsubF->size()) {
      W.insert(finals);
    } else {
      W.insert(QsubF);
    }

    finals->setSuper(Q);
    QsubF->setSuper(Q);

    while (!W.empty()) {
      Set::Ptr R = *W.begin();
      W.erase(W.begin());

      States R_prime_content = set_difference(R->getSuper()->as_set(), R->as_set());
      Set::Ptr R_prime = Set::make(R_prime_content);

      auto P_copy{P};
      bool restart_iteration_over_P = true;
      while (restart_iteration_over_P) {
        restart_iteration_over_P = false;
        while (!P_copy.empty()) {
          auto B {*P_copy.begin()};
          P_copy.erase(P_copy.begin());
          State q, r;
          bool found_in_R = false;
          bool found_in_R_prime = false;
          Predicate DqNotDr;
          Predicate DqNotDr_prime;

          struct XX {
            State q;
            Predicate Dq_R;
            Predicate Dq_R_prime;
          };

          std::vector<XX> Bqs;
          Bqs.reserve(B->size());
          for (auto q : *B) {
            Bqs.push_back({ q, delta(a/*deltaM*/, q, *R), delta(a/*deltaM*/, q, *R_prime)});
          }

          for (auto q : Bqs) {
            for (auto r : Bqs) {
              if (q.q != r.q) {
                DqNotDr = q.Dq_R && !r.Dq_R;

                if (sat(DqNotDr)) {
                  found_in_R = true;
                  break;
                }

                DqNotDr_prime = q.Dq_R_prime && !r.Dq_R_prime;

                if (sat(DqNotDr_prime)) {
                  found_in_R_prime = true;
                  break;
                }
              }
            }
            if (found_in_R || found_in_R_prime) { break; }
          }
          if (found_in_R || found_in_R_prime) {
            Set::Ptr D = Set::make();
            Set::Ptr B_dif_D = Set::make();

            if (found_in_R) {
              for (auto p : *B) {
                if (sat(delta(a/*deltaM*/, p, *R) && DqNotDr)) {
                  D->insert(p);
                } else {
                  B_dif_D->insert(p);
                }
              }
            } else {
              for (auto p : *B) {
                if (sat(delta(a/*deltaM*/, p, *R_prime) && DqNotDr_prime)) {
                  D->insert(p);
                } else {
                  B_dif_D->insert(p);
                }
              }
            }
            // refine P by splitting B
            P.erase(B);
            P.insert(D);
            P.insert(B_dif_D);
            P_copy.insert(D);
            P_copy.insert(B_dif_D);

            auto bi = W.find(B);
            if (bi != W.end()) {
              W.erase(bi);
              W.insert(D);
              W.insert(B_dif_D);

              auto SUPER_B = B->getSuper();
              D->setSuper(SUPER_B);
              B_dif_D->setSuper(SUPER_B);
            } else {
              if (D->size() <= B_dif_D->size()) {
                W.insert(D);
              } else {
                W.insert(B_dif_D);
              }

              D->setSuper(B);
              B_dif_D->setSuper(B);
            }

            //restart_iteration_over_P = true;
            //break; // abort current iteration over P as P was changed
          }
        }
      }
    }
    for (auto qs : P) {
      partition.insert(qs->as_set());
    }
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

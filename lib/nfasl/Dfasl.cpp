#include "rt/RtDfasl.hpp"
#include "nfasl/Dfasl.hpp"
#include "nfasl/Nfasl.hpp"
#include "sat/Sat.hpp"
#include "sat/Transform.hpp"
#include "sat/Simplify.hpp"
#include "boolean/Expr.hpp"
#include "Algo.hpp"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dfasl {
  static void from_json(const json& j, TransitionRule& p) {
    j.at("phi").get_to(p.phi);
    j.at("state").get_to(p.state);
  }

  static void to_json(json& j, const TransitionRule& p) {
    j = json{{"phi", p.phi}, {"state", p.state}};
  }

  void from_json(const json& j, Dfasl& a) {
    j.at("atomicCount").get_to(a.atomicCount);
    j.at("stateCount").get_to(a.stateCount);
    j.at("initial").get_to(a.initial);
    j.at("finals").get_to(a.finals);
    j.at("transitions").get_to(a.transitions);
  }

  void to_json(json& j, const Dfasl& a) {
    j = json{
      {"atomicCount", a.atomicCount},
      {"stateCount",  a.stateCount},
      {"initial",     a.initial},
      {"finals",      a.finals},
      {"transitions", a.transitions}
    };
  }

  std::string pretty(const Dfasl& a) {
    std::ostringstream s;
    constexpr int spaces = 4;
    s << json(a).dump(spaces) << std::endl;
    return s.str();
  }

  typedef std::vector<std::tuple<State, nfasl::States>> Candidates;

  class Builder {
  public:
    Builder(const nfasl::Nfasl& n_, Dfasl& a_)
      : n(n_), a(a_) {
      a.atomicCount = n.atomicCount;
      a.initial = 0;
      a.stateCount = 0;
    }

    State addCandidate(const nfasl::States& qs) {
      assert(!qs.empty());
      auto r = stateMap.insert({ qs, stateMap.size() });
      if (r.second) {
        candidates.push_back({r.first->second, qs});
        a.transitions.resize(stateMap.size());
      }
      return r.first->second;
    }

    void swapCandidates(Candidates& cs) {
      assert(cs.empty());
      std::swap(candidates, cs);
    }

    void addTransitionRule(State source, boolean::Expr phi, State target) {
      assert(source < a.transitions.size());
      a.transitions[source].push_back({ phi, target });
    }

    void finalize() {
      States ini({n.initial});
      a.stateCount = stateMap.size();
      a.initial = stateMap.at(ini);
      for (auto const& v : stateMap) {
        if (set_non_empty_intersection(v.first, n.finals)) {
          a.finals.insert(v.second);
        }
      }
    }

  private:
    const nfasl::Nfasl& n;
    Dfasl& a;
    std::map<nfasl::States, State> stateMap;
    Candidates candidates;
  };

  void deeper(Builder& builder,
              const std::map<nfasl::State, boolean::Expr>& next,
              State sourceNew,
              boolean::Expr upper,
              const nfasl::States& qs) {
    boolean::Expr e0 = upper;
    for (auto q : qs) {
      e0 = e0 && next.at(q);
    }

    e0 = simplify(e0);

    boolean::Expr nextUpper;
    //if (sat(e0)) {
    if (e0 != boolean::Expr::value(false)) {
      State targetNew = builder.addCandidate(qs);
      builder.addTransitionRule(sourceNew, e0, targetNew);

      nextUpper = nnf(!e0);
      // short cut
      // if (!sat(nextUpper)) {
      if (nextUpper == boolean::Expr::value(false)) {
        return;
      }
    } else {
      nextUpper = boolean::Expr::value(true);
    }

    if (qs.size() > 1) {
      for (auto q : qs) {
        States substates{qs};
        substates.erase(q);

        deeper(builder, next, sourceNew, nextUpper, substates);
      }
    }
  }

  void nextState(const nfasl::Nfasl& a,
                 Builder& builder,
                 State sourceNew,
                 const nfasl::States& sources) {
    // prepare mapping to target states
    nfasl::States targets;
    std::map<nfasl::State, boolean::Expr> next;
    for (auto s : sources) {
      for (auto const& rule : a.transitions[s]) {
        auto r = next.insert({rule.state, rule.phi});
        targets.insert(rule.state);
        if (!r.second) {
          r.first->second = r.first->second || rule.phi;
        }
      }
    }

    if (!targets.empty()) {
      deeper(builder, next, sourceNew, boolean::Expr::value(true), targets);
    }
  }

  void toDfasl(const nfasl::Nfasl& a, Dfasl& b) {
    Builder builder(a, b);

    builder.addCandidate({a.initial});
    Candidates candidates;
    builder.swapCandidates(candidates);

    while (!candidates.empty()) {
      for (auto& [u,vs] : candidates) {
        nextState(a, builder, u, vs);
      }
      candidates = Candidates{};
      builder.swapCandidates(candidates);
    }

    builder.finalize();
  }

  void complement(dfasl::Dfasl& a) {
    assert(a.stateCount != 0);

    States finals;
    std::swap(a.finals, finals);

    // make it total
    State newQ = a.stateCount;
    for (State q = 0; q < a.stateCount; ++q) {
      boolean::Expr phi = boolean::Expr::value(false);
      for (auto r : a.transitions[q]) {
        phi = phi || r.phi;
      }
      a.transitions[q].push_back({ !phi, newQ });
    }
    ++a.stateCount;
    a.transitions.resize(a.stateCount);
    a.transitions[newQ].push_back({ boolean::Expr::value(true), newQ });

    // Invert final states
    for (State q = 0; q < a.stateCount; ++q) {
      if (!set_member(finals, q)) {
        a.finals.insert(q);
      }
    }

    assert(set_member(a.finals, newQ));
  }

  void toRt(const Dfasl& u, rt::Dfasl& v) {
    v.atomicCount = u.atomicCount;
    v.stateCount = u.stateCount;
    v.initial = u.initial;
    v.finals.reserve(u.finals.size());
    for (auto q : u.finals) {
      v.finals.insert(q);
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
} // namespace dfasl

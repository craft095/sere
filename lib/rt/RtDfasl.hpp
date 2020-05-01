#ifndef RTDFASL_HPP
#define RTDFASL_HPP

#include <cstdint>
#include <vector>
#include <unordered_set>

#include "rt/RtPredicate.hpp"
#include "Match.hpp"

namespace rt {
  class Dfasl {
  public:
    typedef uint32_t State;
    typedef std::vector<uint8_t> Phi;

    struct StateTransition {
      Phi phi;
      State state;
    };

    typedef std::vector<StateTransition> StateTransitions;

    typedef std::unordered_set<State> States;

    uint16_t atomicCount;
    State stateCount;
    State initial;
    States finals;
    std::vector<StateTransitions> transitions;
  };

  class DfaslContext {
  public:
    DfaslContext (const Dfasl& dfasl_) : dfasl(dfasl_) { reset(); }
    Match getResult() const { return result; }

    void reset();
    void advance(const Names& vars);

  private:
    void fail() { result = Match_Failed; }

    void ok() {
      if (result != Match_Failed) {
        result = Match_Ok;
      }
    }

    void partial() {
      if (result != Match_Failed) {
        result = Match_Partial;
      }
    }

    void checkFinals() {
      if (dfasl.finals.find(currentState) != dfasl.finals.end()) {
        ok();
      } else {
        partial();
      }
    }

  private:
    const Dfasl& dfasl;
    Dfasl::State currentState;

    Match result;
  };

  extern void load(const uint8_t* data, size_t len, Dfasl& dfasl);
  extern void write(const Dfasl& dfasl, std::vector<uint8_t>& data);

} // namespace rt

#endif //RTDFASL_HPP

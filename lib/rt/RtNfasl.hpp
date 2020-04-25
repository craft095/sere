#ifndef RTNFASL_HPP
#define RTNFASL_HPP

#include <cstdint>
#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "rt/RtPredicate.hpp"
#include "Match.hpp"

namespace rt {
  typedef uint16_t State;
  typedef std::vector<uint8_t> Phi;

  struct StateTransition {
    Phi phi;
    State state;
  };

  typedef std::vector<StateTransition> StateTransitions;

  typedef boost::dynamic_bitset<> States;

  class Nfasl {
  public:
    uint16_t atomicCount;
    State stateCount;
    States initials;
    States finals;
    std::vector<StateTransitions> transitions;
  };

  class NfaslContext {
  public:
    NfaslContext (const Nfasl& nfasl_) : nfasl(nfasl_) { reset(); }
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
      if (currentStates.intersects(nfasl.finals)) {
        ok();
      } else {
        partial();
      }
    }

  private:
    const Nfasl& nfasl;
    States currentStates;

    Match result;
  };

  extern void load(const uint8_t* data, size_t len, Nfasl& nfasl);
  extern void write(const Nfasl& nfasl, std::vector<uint8_t>& data);

} // namespace rt

#endif //RTNFASL_HPP

#ifndef RTNFASL_HPP
#define RTNFASL_HPP

#include <cstdint>
#include <vector>
#include <boost/dynamic_bitset.hpp>

namespace rtnfasl {
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

  extern void load(const uint8_t* data, size_t len, Nfasl& nfasl);
  extern void write(const Nfasl& nfasl, std::vector<uint8_t>& data);

}

#endif //RTNFASL_HPP

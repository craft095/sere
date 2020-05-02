#ifndef RTNFASL_HPP
#define RTNFASL_HPP

#include "rt/RtPredicate.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"
#include "Match.hpp"

#include <cstdint>
#include <vector>
#include <boost/dynamic_bitset.hpp>

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

  class NfaslContext : public Executor {
  public:
    NfaslContext (std::shared_ptr<Nfasl> nfasl_) : nfasl(nfasl_) { reset(); }
    Match getResult() const override { return result; }

    void reset() override;
    void advance(const Names& vars) override;

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
      if (currentStates.intersects(nfasl->finals)) {
        ok();
      } else {
        partial();
      }
    }

  private:
    std::shared_ptr<Nfasl> nfasl;
    States currentStates;

    Match result;
  };

  class NfaslLoad : public LoadCallback {
  public:
    std::shared_ptr<Executor> load(Loader& loader) override;
  };

  extern void write(const Nfasl& nfasl, std::vector<uint8_t>& data);

} // namespace rt

#endif //RTNFASL_HPP

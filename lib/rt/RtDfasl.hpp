#ifndef RTDFASL_HPP
#define RTDFASL_HPP

#include <cstdint>
#include <vector>
#include <unordered_set>

#include "rt/RtPredicate.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"
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

  class DfaslContext : public Executor {
  public:
    DfaslContext (std::shared_ptr<Dfasl> dfasl_) : dfasl(dfasl_) { reset(); }
    Match getResult() const override { return result; }

    void reset() override ;
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
      if (dfasl->finals.find(currentState) != dfasl->finals.end()) {
        ok();
      } else {
        partial();
      }
    }

  private:
    std::shared_ptr<Dfasl> dfasl;
    Dfasl::State currentState;

    Match result;
  };

  class DfaslExecutorFactory : public LoadCallback {
  public:
    std::shared_ptr<Executor> load(Loader& loader) override;
  };

  extern void write(const Dfasl& dfasl, std::vector<uint8_t>& data);

} // namespace rt

#endif //RTDFASL_HPP

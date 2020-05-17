#ifndef RTNFASL_HPP
#define RTNFASL_HPP

#include "rt/RtPredicate.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"
#include "rt/RtContext.hpp"
#include "Match.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>
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

  typedef std::unordered_map<State, RtContext> StateMap;

  class NfaslExtendedContext : public ExtendedExecutor {
  public:
    NfaslExtendedContext (std::shared_ptr<Nfasl> nfasl_) : nfasl(nfasl_) { reset(); }
    const ExtendedMatch& getResult() const override {
      return result;
    }

    void reset() override;
    void advance(const Names& vars) override;

  private:
    void initials(States& qs, StateMap& map);
    void finals();

    size_t horizon;
    std::shared_ptr<Nfasl> nfasl;
    States currentStates;
    StateMap currentContext;
    ExtendedMatch result;
  };

  extern std::shared_ptr<Nfasl> loadNfasl(Loader& loader);
  extern void write(const Nfasl& nfasl, std::vector<uint8_t>& data);

  class NfaslExecutorFactory : public LoadCallback {
  public:
    std::shared_ptr<Executor> load(Loader& loader) override {
      return std::make_shared<NfaslContext>(loadNfasl(loader));
    }
  };

  class NfaslExtendedExecutorFactory : public LoadExtendedCallback {
  public:
    std::shared_ptr<ExtendedExecutor> load(Loader& loader) override {
      return std::make_shared<NfaslExtendedContext>(loadNfasl(loader));
    }
  };

} // namespace rt

#endif //RTNFASL_HPP

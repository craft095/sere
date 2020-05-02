#include "rt/RtDfasl.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"

namespace rt {

  static void loadStates(Loader& loader, Dfasl::States& states) {
    Dfasl::State count;
    loader.readValue(count);
    states.reserve(count);
    while (count--) {
      Dfasl::State q;
      loader.readValue(q);
      states.insert(q);
    }
  }

  static void loadStateTransition(Loader& loader, Dfasl::StateTransition& str) {
    loader.loadPredicate(str.phi);
    loader.readValue(str.state);
  }

  static void loadStateTransitions(Loader& loader, Dfasl::StateTransitions& strs) {
    uint32_t trsCount;
    loader.readValue(trsCount);
    strs.resize(trsCount);
    for (auto& str : strs) {
      loadStateTransition(loader, str);
    }
  }

  std::shared_ptr<Executor> DfaslLoad::load(Loader& loader) {
    std::shared_ptr<Dfasl> dfasl = std::make_shared<Dfasl>();
    const Header& hdr = loader.getHeader();
    dfasl->atomicCount = hdr.atomicCount;
    dfasl->stateCount = hdr.stateCount;
    loader.readValue(dfasl->initial);
    loadStates(loader, dfasl->finals);
    dfasl->transitions.resize(dfasl->stateCount);

    for (auto& t : dfasl->transitions) {
      loadStateTransitions(loader, t);
    }

    return std::make_shared<DfaslContext>(dfasl);
  }

  class DfaslSaver : public Saver {
  private:
    void saveStates(const Dfasl::States& states) {
      writeValue(states.size());
      for (auto q : states) {
        writeValue(q);
      }
    }

    void saveStateTransition(const Dfasl::StateTransition& str) {
      savePredicate(str.phi);
      writeValue(str.state);
    }

    void saveStateTransitions(const Dfasl::StateTransitions& strs) {
      writeValue(strs.size());
      for (auto const& str : strs) {
        saveStateTransition(str);
      }
    }

  public:
    DfaslSaver(std::vector<uint8_t>& data) : Saver(data) {}

    void save(const Dfasl& dfasl) {
      Header hdr;
      hdr.magic = magic;
      hdr.kind = Kind::DFASL;
      hdr.atomicCount = dfasl.atomicCount;
      hdr.stateCount = dfasl.stateCount;
      writeValue(hdr);
      writeValue(dfasl.initial);
      saveStates(dfasl.finals);
      for (auto const& t : dfasl.transitions) {
        saveStateTransitions(t);
      }
    }
  };

  void write(const Dfasl& dfasl, std::vector<uint8_t>& data) {
    DfaslSaver(data).save(dfasl);
  }

  void DfaslContext::reset() {
    if (dfasl->finals.size() == 0) {
      fail();
    } else {
      currentState = dfasl->initial;
      checkFinals();
    }
  }

  void DfaslContext::advance(const rt::Names& vars) {
    bool advanced = false;

    Dfasl::State nextState;
    const Dfasl::StateTransitions& trs = dfasl->transitions[currentState];
    for (auto& tr : trs) {
      if (eval(vars, &tr.phi[0], tr.phi.size())) {
        advanced = true;
        nextState = tr.state;
        break;
      }
    }

    currentState = nextState;
    if (advanced) {
      checkFinals();
    } else {
      fail();
    }
  }

} //namespace rt

#include "rt/RtNfasl.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"

#include <memory.h>

namespace rt {

  static void loadStates(Loader& loader, States& states) {
    State count;
    loader.readValue(count);
    states.resize(count);
    while (count--) {
      State q;
      loader.readValue(q);
      states.set(q, true);
    }
  }

  static void loadStateTransition(Loader& loader, StateTransition& str) {
    loader.loadPredicate(str.phi);
    loader.readValue(str.state);
  }

  static void loadStateTransitions(Loader& loader, StateTransitions& strs) {
    uint32_t trsCount;
    loader.readValue(trsCount);
    strs.resize(trsCount);
    for (auto& str : strs) {
      loadStateTransition(loader, str);
    }
  }

  std::shared_ptr<Executor> NfaslLoad::load(Loader& loader) {
    std::shared_ptr<Nfasl> nfasl = std::make_shared<Nfasl>();
    const Header& hdr = loader.getHeader();
    nfasl->atomicCount = hdr.atomicCount;
    nfasl->stateCount = hdr.stateCount;
    loadStates(loader, nfasl->initials);
    loadStates(loader, nfasl->finals);
    nfasl->transitions.resize(nfasl->stateCount);

    for (auto& t : nfasl->transitions) {
      loadStateTransitions(loader, t);
    }

    return std::make_shared<NfaslContext>(nfasl);
  }

  class NfaslSaver : public Saver {
  private:
    void saveStates(const States& states) {
      writeValue(states.count());
      for (State q = 0; q < states.size(); ++q) {
        if (states.test(q)) {
          writeValue(q);
        }
      }
    }

    void saveStateTransition(const StateTransition& str) {
      savePredicate(str.phi);
      writeValue(str.state);
    }

    void saveStateTransitions(const StateTransitions& strs) {
      writeValue(strs.size());
      for (auto const& str : strs) {
        saveStateTransition(str);
      }
    }

  public:
    NfaslSaver(std::vector<uint8_t>& data) : Saver(data) {}

    void save(const Nfasl& nfasl) {
      Header hdr;
      hdr.magic = magic;
      hdr.atomicCount = nfasl.atomicCount;
      hdr.stateCount = nfasl.stateCount;
      writeValue(hdr);
      saveStates(nfasl.initials);
      saveStates(nfasl.finals);
      for (auto const& t : nfasl.transitions) {
        saveStateTransitions(t);
      }
    }
  };

  void write(const Nfasl& nfasl, std::vector<uint8_t>& data) {
    NfaslSaver(data).save(nfasl);
  }

  void NfaslContext::reset() {
    if (nfasl->finals.count() == 0) {
      fail();
    } else {
      currentStates = nfasl->initials;
      checkFinals();
    }
  }

  void NfaslContext::advance(const rt::Names& vars) {
    bool advanced = false;
    // iterate over current state
    rt::Names nextStates;
    nextStates.resize(nfasl->stateCount);
    for (size_t q = currentStates.find_first();
         q != States::npos;
         q = currentStates.find_next(q)) {
      const StateTransitions& trs = nfasl->transitions[q];
      for (auto& tr : trs) {
        if (eval(vars, &tr.phi[0], tr.phi.size())) {
          advanced = true;
          nextStates.set(tr.state);
        }
      }
    }
    std::swap(currentStates, nextStates);
    if (advanced) {
      checkFinals();
    } else {
      fail();
    }
  }

} //namespace rt

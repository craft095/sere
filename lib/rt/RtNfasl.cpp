#include "rt/RtNfasl.hpp"
#include "rt/Executor.hpp"
#include "rt/Loader.hpp"
#include "rt/Saver.hpp"

#include <memory.h>

namespace rt {

  static void loadStates(Loader& loader, States& states) {
    uint32_t size;
    uint32_t count;
    loader.readValue(size);
    loader.readValue(count);
    states.resize(size);
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

  std::shared_ptr<Nfasl> loadNfasl(Loader& loader) {
    std::shared_ptr<Nfasl> nfasl = std::make_shared<Nfasl>();
    const Header& hdr = loader.getHeader();
    nfasl->atomicCount = hdr.atomicCount;
    nfasl->stateCount = hdr.stateCount;
    loadStates(loader, nfasl->initials);
    loadStates(loader, nfasl->finals);
    nfasl->transitions.resize(nfasl->stateCount);
    return nfasl;
  }

  class NfaslSaver : public Saver {
  private:
    void saveStates(const States& states) {
      writeValue(uint32_t(states.size()));
      writeValue(uint32_t(states.count()));
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
      writeValue(uint32_t(strs.size()));
      for (auto const& str : strs) {
        saveStateTransition(str);
      }
    }

  public:
    NfaslSaver(std::vector<uint8_t>& data) : Saver(data) {}

    void save(const Nfasl& nfasl) {
      Header hdr;
      hdr.magic = magic;
      hdr.kind = Kind::NFASL;
      hdr.atomicCount = nfasl.atomicCount;
      hdr.stateCount = nfasl.stateCount;
      writeValue(hdr);
      saveStates(nfasl.initials);
      saveStates(nfasl.finals);
      assert(nfasl.transitions.size() == nfasl.stateCount);
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

  void NfaslExtendedContext::initials(States& qs, StateMap& map) {
    for (size_t q = nfasl->initials.find_first();
         q != States::npos;
         q = nfasl->initials.find_next(q)) {
      qs.set(q);
      map[q].started();
    }
  }

  void NfaslExtendedContext::finals() {
    if (nfasl->finals.count() == 0) {
      result.match = Match_Failed;
      return;
    }
    RtContext ctx;
    for (size_t q = nfasl->finals.find_first();
         q != States::npos;
         q = nfasl->finals.find_next(q)) {
      auto i = currentContext.find(q);
      if (i != currentContext.end()) {
        ctx.merge(i->second);
      }
    }
    if (ctx.defined()) {
      result.match = Match_Ok;
      result.ok.shortest = ctx.shortest;
      result.ok.longest = ctx.longest;
      result.ok.horizon = horizon;
    } else {
      result.match = Match_Partial;
      result.partial.horizon = horizon;
    }
  }

  void NfaslExtendedContext::reset() {
    horizon = 0;
    currentStates.resize(nfasl->stateCount);
    initials(currentStates, currentContext);
    finals();
  }

  void NfaslExtendedContext::advance(const rt::Names& vars) {
    bool advanced = false;
    rt::Names nextStates;
    StateMap nextContext;
    nextStates.resize(nfasl->stateCount);
    initials(nextStates, nextContext);
    for (size_t q = currentStates.find_first();
         q != States::npos;
         q = currentStates.find_next(q)) {
      bool advancedState = false;
      auto const& advancedCtx = RtContext::advance(currentContext[q]);
      const StateTransitions& trs = nfasl->transitions[q];
      for (auto& tr : trs) {
        if (eval(vars, &tr.phi[0], tr.phi.size())) {
          advancedState = true;
          nextStates.set(tr.state);
          nextContext[tr.state].merge(advancedCtx);
        }
      }
      if (advancedState) {
        advanced = true;
        horizon = std::max(horizon, advancedCtx.longest);
      }
    }
    std::swap(currentStates, nextStates);
    std::swap(currentContext, nextContext);
    if (!advanced) {
      horizon = 0;
    }
    finals();
  }

} //namespace rt

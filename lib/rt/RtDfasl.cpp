#include "rt/RtDfasl.hpp"

#include <memory.h>

namespace rt {

  struct Header {
    uint32_t magic;
    uint16_t atomicCount;
    uint16_t stateCount;
  } __attribute__((packed));

  constexpr uint32_t magic = 0x8a33b462;

  class Loader {
  private:
    const uint8_t* begin;
    const uint8_t* end;
    const uint8_t* curr;

    template <typename T>
    void readValue(T& t) {
      assert(curr + sizeof(T) <= end);
      t = *reinterpret_cast<const T*>(curr);
      curr += sizeof(T);
    }

    void readData(uint8_t* d, size_t len) {
      assert(curr + len <= end);
      memcpy(d, curr, len);
      curr += len;
    }

    void loadStates(Dfasl::States& states) {
      Dfasl::State count;
      readValue(count);
      states.reserve(count);
      while (count--) {
        Dfasl::State q;
        readValue(q);
        states.insert(q);
      }
    }

    void loadPhi(Dfasl::Phi& phi) {
      uint32_t size;
      readValue(size);
      phi.resize(size);
      readData(&phi[0], sizeof(phi[0])*size);
    }

    void loadStateTransition(Dfasl::StateTransition& str) {
      loadPhi(str.phi);
      readValue(str.state);
    }

    void loadStateTransitions(Dfasl::StateTransitions& strs) {
      uint32_t trsCount;
      readValue(trsCount);
      strs.resize(trsCount);
      for (auto& str : strs) {
        loadStateTransition(str);
      }
    }
  public:
    Loader(const uint8_t* data, size_t len)
      : begin(data), end(data + len), curr(data) {
    }

    void load(Dfasl& dfasl) {
      Header hdr;
      readValue(hdr);
      dfasl.atomicCount = hdr.atomicCount;
      dfasl.stateCount = hdr.stateCount;
      readValue(dfasl.initial);
      loadStates(dfasl.finals);
      dfasl.transitions.resize(dfasl.stateCount);

      for (auto& t : dfasl.transitions) {
        loadStateTransitions(t);
      }
    }
  };

  void load(const uint8_t* data, size_t len, Dfasl& dfasl) {
    Loader(data, len).load(dfasl);
  }

  class Saver {
  private:
    std::vector<uint8_t> data;

    void writeData(const uint8_t* d, size_t len) {
      data.insert(data.end(), d, d + len);
    }

    template <typename T>
    void writeValue(const T& t) {
      writeData((const uint8_t*)(&t), sizeof(t));
    }

    void saveStates(const Dfasl::States& states) {
      writeValue(states.size());
      for (auto q : states) {
        writeValue(q);
      }
    }

    void savePhi(const Dfasl::Phi& phi) {
      writeValue(phi.size());
      writeData(&phi[0], sizeof(phi[0])*phi.size());
    }

    void saveStateTransition(const Dfasl::StateTransition& str) {
      savePhi(str.phi);
      writeValue(str.state);
    }

    void saveStateTransitions(const Dfasl::StateTransitions& strs) {
      writeValue(strs.size());
      for (auto const& str : strs) {
        saveStateTransition(str);
      }
    }

  public:
    Saver(std::vector<uint8_t>& data_) : data(data_) {}

    void save(const Dfasl& dfasl) {
      Header hdr;
      hdr.magic = magic;
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
    Saver(data).save(dfasl);
  }

  void DfaslContext::reset() {
    if (dfasl.finals.size() == 0) {
      fail();
    } else {
      currentState = dfasl.initial;
      checkFinals();
    }
  }

  void DfaslContext::advance(const rt::Names& vars) {
    bool advanced = false;

    Dfasl::State nextState;
    const Dfasl::StateTransitions& trs = dfasl.transitions[currentState];
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

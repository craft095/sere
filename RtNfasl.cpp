#include "RtNfasl.hpp"

#include <memory.h>

namespace rtnfasl {

  struct Header {
    uint32_t magic;
    uint16_t atomicCount;
    uint16_t stateCount;
  } __attribute__((packed));

  constexpr uint32_t magic = 0x82337462;

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

    void loadStates(States& states) {
      State count;
      readValue(count);
      states.resize(count);
      while (count--) {
        State q;
        readValue(q);
        states.set(q, true);
      }
    }

    void loadPhi(Phi& phi) {
      uint32_t size;
      readValue(size);
      phi.resize(size);
      readData(&phi[0], sizeof(phi[0])*size);
    }

    void loadStateTransition(StateTransition& str) {
      loadPhi(str.phi);
      readValue(str.state);
    }

    void loadStateTransitions(StateTransitions& strs) {
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

    void load(Nfasl& nfasl) {
      Header hdr;
      readValue(hdr);
      nfasl.atomicCount = hdr.atomicCount;
      nfasl.stateCount = hdr.stateCount;
      loadStates(nfasl.initials);
      loadStates(nfasl.finals);
      nfasl.transitions.resize(nfasl.stateCount);

      for (auto& t : nfasl.transitions) {
        loadStateTransitions(t);
      }
    }
  };

  void load(const uint8_t* data, size_t len, Nfasl& nfasl) {
    Loader(data, len).load(nfasl);
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

    void saveStates(const States& states) {
      writeValue(states.count());
      for (State q = 0; q < states.size(); ++q) {
        if (states.test(q)) {
          writeValue(q);
        }
      }
    }

    void savePhi(const Phi& phi) {
      writeValue(phi.size());
      writeData(&phi[0], sizeof(phi[0])*phi.size());
    }

    void saveStateTransition(const StateTransition& str) {
      savePhi(str.phi);
      writeValue(str.state);
    }

    void saveStateTransitions(const StateTransitions& strs) {
      writeValue(strs.size());
      for (auto const& str : strs) {
        saveStateTransition(str);
      }
    }

  public:
    Saver(std::vector<uint8_t>& data_) : data(data_) {}

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
    Saver(data).save(nfasl);
  }

}

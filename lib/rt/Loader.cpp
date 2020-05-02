#include "rt/Loader.hpp"
#include "rt/Format.hpp"
#include "rt/RtNfasl.hpp"
#include "rt/RtDfasl.hpp"

#include <memory.h>

namespace rt {
  Loader::Loader(const uint8_t* data, size_t len)
    : begin(data), end(data + len), curr(data) {
  }

  void Loader::readData(uint8_t* d, size_t len) {
    ensure(curr + len <= end);
    memcpy(d, curr, len);
    curr += len;
  }

  void Loader::loadPredicate(Predicate& phi) {
    uint32_t size;
    readValue(size);
    phi.resize(size);
    readData(&phi[0], sizeof(phi[0])*size);
  }

  void Loader::ensure(bool cond) {
    if (!cond) {
      throw LoadingFailed{};
    }
  }

  void Loader::addCallback(Kind kind, LoadCallback* callback) {
    callbacks[kind] = callback;
  }

  ExecutorPtr Loader::load(const std::vector<uint8_t>& data) {
    return load(&data[0], data.size());
  }

  const Header& Loader::getHeader() const {
    return header;
  }

  static void initializeCallbacks() {
    static NfaslLoad nfaslLoad;
    static DfaslLoad dfaslLoad;
    Loader::addCallback(Kind::NFASL, &nfaslLoad);
    Loader::addCallback(Kind::DFASL, &dfaslLoad);
  }

  ExecutorPtr Loader::load(const uint8_t* data, size_t len) {
    if (callbacks.size() == 0) {
      initializeCallbacks();
    }

    Loader loader{data, len};

    loader.readValue(loader.header);

    auto r = callbacks.find(static_cast<Kind>(loader.header.kind));
    loader.ensure(r != callbacks.end());
    return r->second->load(loader);
  }

  std::map<Kind, LoadCallback*> Loader::callbacks;

} //namespace rt

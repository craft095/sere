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

  void Loader::addCallback(Kind kind, LoadExtendedCallback* callback) {
    extendedCallbacks[kind] = callback;
  }

  ExecutorPtr Loader::load(const std::vector<uint8_t>& data) {
    return load(&data[0], data.size());
  }

  ExtendedExecutorPtr Loader::loadExtended(const std::vector<uint8_t>& data) {
    return loadExtended(&data[0], data.size());
  }

  const Header& Loader::getHeader() const {
    return header;
  }

  static void initializeCallbacks() {
    static NfaslExecutorFactory nfaslFactory;
    static NfaslExtendedExecutorFactory nfaslExtendedFactory;
    static DfaslExecutorFactory dfaslFactory;
    Loader::addCallback(Kind::NFASL, &nfaslFactory);
    Loader::addCallback(Kind::NFASL, &nfaslExtendedFactory);
    Loader::addCallback(Kind::DFASL, &dfaslFactory);
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

  ExtendedExecutorPtr Loader::loadExtended(const uint8_t* data, size_t len) {
    if (extendedCallbacks.size() == 0) {
      initializeCallbacks();
    }

    Loader loader{data, len};

    loader.readValue(loader.header);

    auto r = extendedCallbacks.find(static_cast<Kind>(loader.header.kind));
    loader.ensure(r != extendedCallbacks.end());
    return r->second->load(loader);
  }

  std::map<Kind, LoadCallback*> Loader::callbacks;
  std::map<Kind, LoadExtendedCallback*> Loader::extendedCallbacks;

} //namespace rt

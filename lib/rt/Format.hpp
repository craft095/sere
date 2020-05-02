#ifndef RTFORMAT_HPP
#define RTFORMAT_HPP

#include <vector>
#include <cstdint>

namespace rt {
  typedef std::vector<uint8_t> Predicate;

  enum Kind : uint16_t {
    NFASL = 0,
    DFASL = 1,
    NFA = 2,
    DFA = 3,
  };

  struct Header {
    uint32_t magic;
    uint16_t kind;
    uint16_t atomicCount;
    uint16_t stateCount;
  } __attribute__((packed));

  constexpr uint32_t magic = 0x82337462;

} //namespace rt

#endif // RTFORMAT_HPP

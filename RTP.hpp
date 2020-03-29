#ifndef RTP_HPP
#define RTP_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <vector>

#include "Language.hpp"

class BoolExpr;

namespace rtp {
  typedef uint16_t Offset;
  class BitSet {
    uint64_t data;
  public:
    BitSet() : data{0} {}

    int get(size_t k) const {
      assert(k < sizeof(data)*8);
      return (data >> k) & 0x1;
    }

    void set(size_t k, int v) {
      uint64_t mask = 1 << k;
      data = (data & ~mask) | (v << k);
    }
  };

  enum class Code : uint8_t {
    False = 0,
    True = 1,
    Name = 2,
    Not = 3,
    And = 4,
    Or = 5
  };

  static constexpr uint32_t magic = 0xec342a4f;

  class Evaluator {
  public:
    Evaluator(const BitSet& names_, const uint8_t* data_, size_t len_)
      : names(names_), data(data_), len(len_), eip(data_) {}


    bool eval() {
      auto m = decltype(magic){0};
      readValue(m);
      assert(m == magic);
      return eval0();
    }

  private:
    template <typename T>
    void readValue(T& t) {
      assert(len >= (size_t)(eip + sizeof(T) - data));
      t = *reinterpret_cast<const T*>(eip);
      eip += sizeof(T);
    }

    int eval0() {
      Code c;

      readValue(c);
      switch (c) {
      case Code::False:
        return 0;
      case Code::True:
        return 1;
      case Code::Name:
        Offset off;
        readValue(off);
        return names.get(off);
      case Code::Not:
        return eval0() ^ 1;
      case Code::And:
        return eval0() && eval0();
      case Code::Or:
        return eval0() || eval0();
      }
      assert(false);
    }

  private:
    const BitSet& names;
    const uint8_t* data;
    size_t len;

    const uint8_t* eip;
  };

  inline bool eval(const BitSet& names, const uint8_t* data, size_t len) {
    return Evaluator(names, data, len).eval();
  }

  extern void toRTP(BoolExpr& expr,
                    std::vector<uint8_t>& data);
}

#endif // RTP_HPP

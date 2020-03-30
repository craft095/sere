#ifndef TESTZ3_HPP
#define TESTZ3_HPP

#include "RTP.hpp"

namespace z3 {
  class expr;
}

extern z3::expr letterToZex(const rtp::Names& l);

extern bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_);
extern bool evalWithImply0(const Letter& l, const z3::expr& expr_);

#endif //TESTZ3_HPP

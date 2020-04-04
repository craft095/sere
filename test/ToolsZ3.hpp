#ifndef TOOLSZ3_HPP
#define TOOLSZ3_HPP

#include "rt/RtPredicate.hpp"

namespace z3 {
  class expr;
}

extern z3::expr letterToZex(const rt::Names& l);

extern bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_);
extern bool evalWithImply0(const rt::Names& l, const z3::expr& expr_);

#endif //TOOLSZ3_HPP

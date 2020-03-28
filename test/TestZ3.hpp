#ifndef TESTZ3_HPP
#define TESTZ3_HPP

namespace z3 {
  class expr;
}

class Letter;

extern z3::expr letterToZex(const Letter& l);

extern bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_);
extern bool evalWithImply0(const Letter& l, const z3::expr& expr_);

#endif //TESTZ3_HPP

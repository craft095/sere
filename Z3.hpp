#ifndef Z3_HPP
#define Z3_HPP

#include "z3++.h"

namespace z3 {
  class expr;
}

class Letter;
class BoolExpr;

extern z3::expr letterToZex(const Letter& l);

extern z3::expr boolSereToZex(BoolExpr& be);

extern bool prove(const z3::expr& e);

extern bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_);

#endif //Z3_HPP

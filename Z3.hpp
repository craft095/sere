#ifndef Z3_HPP
#define Z3_HPP

#include "c++/z3++.h"
#include <nlohmann/json.hpp>

namespace z3 {
  class expr;
}

using json = nlohmann::json;

namespace z3 {
  inline void to_json(json& j, const expr& e) {
    j = e.get_string();
  }
}

class Letter;
class BoolExpr;

extern z3::expr letterToZex(const Letter& l);

extern z3::expr boolSereToZex(BoolExpr& be);

extern bool prove(const z3::expr& e);
extern bool satisfiable(const z3::expr& e);

extern bool evalWithImply(const z3::expr& vars_, const z3::expr& expr_);
extern bool evalWithImply0(const Letter& l, const z3::expr& expr_);

#endif //Z3_HPP

#ifndef Z3_HPP
#define Z3_HPP

#include "z3++.h"
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

class BoolExpr;

extern z3::context theContext;

extern z3::expr boolSereToZex(BoolExpr& be);

extern bool prove(const z3::expr& e);
extern bool satisfiable(const z3::expr& e);

#endif //Z3_HPP

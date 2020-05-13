#ifndef SAT_SAT_HPP
#define SAT_SAT_HPP

#include "boolean/Expr.hpp"

class BoolExpr;
extern bool sat(BoolExpr& expr);

extern bool sat(boolean::Expr expr);
extern bool prove(boolean::Expr expr);

#endif // SAT_SAT_HPP

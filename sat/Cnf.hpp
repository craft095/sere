#ifndef CNF_HPP
#define CNF_HPP

#include "boolean/Expr.hpp"

class BoolExpr;
extern bool sat(boolean::Expr expr);
extern bool sat(BoolExpr& expr);

#endif // CNF_HPP

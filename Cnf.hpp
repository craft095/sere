#ifndef CNF_HPP
#define CNF_HPP

#include "BoolExpr.hpp"

class BoolExpr;
extern bool sat(expr::Expr expr);
extern bool sat(BoolExpr& expr);

#endif // CNF_HPP

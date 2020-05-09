#ifndef CNF_HPP
#define CNF_HPP

#include "boolean/Expr.hpp"

class BoolExpr;
extern bool sat(BoolExpr& expr);

extern boolean::Expr simplify(boolean::Expr expr0);
extern boolean::Expr nnf(boolean::Expr expr0);
extern bool sat(boolean::Expr expr);

#endif // CNF_HPP

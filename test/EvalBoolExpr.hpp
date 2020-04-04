#ifndef EVALBOOLEXPR_HPP
#define EVALBOOLEXPR_HPP

#include "Letter.hpp"

class BoolExpr;

extern bool evalBool(BoolExpr& expr, const Letter& letter);
extern bool evalBoolZ3(BoolExpr& expr, const Letter& letter);

#endif //EVALBOOLEXPR_HPP

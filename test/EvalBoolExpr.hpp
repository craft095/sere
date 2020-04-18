#ifndef EVALBOOLEXPR_HPP
#define EVALBOOLEXPR_HPP

#include "test/Letter.hpp"

class BoolExpr;

extern bool evalBool(BoolExpr& expr, const Letter& letter);
extern bool evalBoolZ3(BoolExpr& expr, const Letter& letter);

#endif //EVALBOOLEXPR_HPP

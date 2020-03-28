#ifndef EVALSERE_HPP
#define EVALSERE_HPP

#include "Letter.hpp"

enum Match {
  Match_Ok,
  Match_Partial,
  Match_Failed,
};

class SereExpr;

extern Match evalSere(SereExpr& expr, const Word& word);
extern bool evalBool(BoolExpr& expr, const Letter& letter);
extern bool evalBoolZ3(BoolExpr& expr, const Letter& letter);

#endif //EVALSERE_HPP

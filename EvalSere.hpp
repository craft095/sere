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

#endif //EVALSERE_HPP

#ifndef TEST_COMPAREEXPRS_HPP
#define TEST_COMPAREEXPRS_HPP

#include "test/Tools.hpp"
#include "test/Letter.hpp"
#include "test/GenLetter.hpp"
#include "test/EvalRt.hpp"

#include "Match.hpp"
#include "ast/Parser.hpp"
#include "nfasl/Nfasl.hpp"
#include "nfasl/BisimNfasl.hpp"
#include "rt/RtNfasl.hpp"

#define COMPARE_EXPRS0(r0, r1)                                          \
  {                                                                     \
    rt::Nfasl rtNfasl0, rtNfasl1;                                       \
    prepareExprs(r0, r1, rtNfasl0, rtNfasl1);                           \
                                                                        \
    auto atoms = rtNfasl0.atomicCount;                                  \
    for (size_t cnt = 0; cnt < 100; ++cnt) {                            \
      auto word0 = WordGenerator::make(atoms, 0, 8);                    \
      Match res0 = evalRtNfasl(rtNfasl0, word0);                        \
      Match res1 = evalRtNfasl(rtNfasl1, word0);                        \
                                                                        \
      CHECK(res0 == res1);                                              \
    }                                                                   \
  }

extern void prepareExpr(Ptr<SereExpr> expr, rt::Nfasl& rtNfasl);
extern void prepareExprs(parser::ParseResult r0,
                         parser::ParseResult r1,
                         rt::Nfasl& rtNfasl0,
                         rt::Nfasl& rtNfasl1);

#endif //TEST_COMPAREEXPRS_HPP

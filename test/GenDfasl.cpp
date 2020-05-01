#include <memory>

#include "test/Letter.hpp"
#include "test/GenExpr.hpp"
#include "test/GenDfasl.hpp"

#include "nfasl/Dfasl.hpp"
#include "Algo.hpp"

using namespace dfasl;

namespace dfasl {
  static State makeState(size_t states) {
    return choose((State)0, states - 1);
  }

  static States makeStates(size_t mn, size_t mx, size_t states) {
    auto f = [states]() { return makeState(states); };
    return set_of<State>(mn, mx, f);
  }

  static TransitionRule makeRule(boolean::Expr excl,
                                 size_t depth,
                                 size_t atoms,
                                 size_t states) {
    return { ExprGenerator::make(depth, atoms) && excl, makeState(states) };
  }

  static TransitionRules
  makeTransitionRules(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
    size_t trsCount = choose((size_t)0, maxTrs + 1);
    TransitionRules trs;
    trs.reserve(trsCount);

    boolean::Expr excl{boolean::Expr::value(true)};
    for (size_t t = 0; t < trsCount; ++t) {
      boolean::Expr phi = ExprGenerator::make(depth, atoms);
      State q = makeState(states);
      trs.push_back({ phi && excl, q });
      excl = excl && !phi;
    }

    return trs;
  }
}

Ptr<Dfasl>
makeDfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
  Ptr<Dfasl> a = std::make_shared<Dfasl>();
  a->atomicCount = atoms;
  a->stateCount = states;
  a->initial = makeState(states);
  a->finals = makeStates(0, states, states);
  a->transitions.reserve(states);
  for (size_t s = 0; s < states; ++s) {
    a->transitions.push_back(makeTransitionRules(depth, atoms, states, maxTrs));
  }
  return a;
}

class DfaslGenerator : public Catch2::IGenerator <Ptr<Dfasl>> {
public:
  DfaslGenerator(size_t depth, size_t atoms_, size_t states_, size_t maxTrs_)
    : maxDepth(depth), atoms(atoms_), states(states_), maxTrs(maxTrs_) {
    next();
  }

  Ptr<Dfasl> const& get() const override {
    return value;
  }

  bool next() override {
    value = makeDfasl(maxDepth, atoms, states, maxTrs);
    return true;
  }

private:
  size_t maxDepth;
  size_t atoms;
  size_t states;
  size_t maxTrs;
  Ptr<Dfasl> value;
};

#if 1
Catch2::GeneratorWrapper<Ptr<Dfasl>>
genDfasl(size_t depth, size_t atomics, size_t states, size_t maxTrs) {
  return Catch2::GeneratorWrapper<Ptr<Dfasl>>
    (std::make_unique<DfaslGenerator>
     (depth, atomics, states, maxTrs));
}
#endif

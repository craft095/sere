#include <memory>

#include "Nfasl.hpp"
#include "Letter.hpp"
#include "GenBoolExpr.hpp"
#include "Algo.hpp"

#include "GenNfasl.hpp"

using namespace nfasl;

namespace nfasl {
  static State makeState(size_t states) {
    return choose((State)0, states - 1);
  }

  static States makeStates(size_t mn, size_t mx, size_t states) {
    auto f = [states]() { return makeState(states); };
    return set_of<State>(mn, mx, f);
  }

  static TransitionRule makeRule(size_t depth, size_t atoms, size_t states) {
    return { BoolExprGenerator::make(depth, atoms), makeState(states) };
  }

  static TransitionRules
  makeTransitionRules(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
    auto g = [depth, atoms, states]() { return makeRule(depth, atoms, states); };
    return vector_of<TransitionRule>(0, maxTrs, g);
  }
}


Ptr<Nfasl>
makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs) {
  Ptr<Nfasl> a = std::make_shared<Nfasl>();
  for (size_t ix = 0; ix < atoms; ++ix) {
    a->atomics.insert(make_varName(ix));
  }
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

class NfaslGenerator : public Catch2::IGenerator <Ptr<Nfasl>> {
public:
  NfaslGenerator(size_t depth, size_t atoms_, size_t states_, size_t maxTrs_)
    : maxDepth(depth), atoms(atoms_), states(states_), maxTrs(maxTrs_) {
    next();
  }

  Ptr<Nfasl> const& get() const override {
    return value;
  }

  bool next() override {
    value = makeNfasl(maxDepth, atoms, states, maxTrs);
    return true;
  }

private:
  size_t maxDepth;
  size_t atoms;
  size_t states;
  size_t maxTrs;
  Ptr<Nfasl> value;
};

#if 1
Catch2::GeneratorWrapper<Ptr<Nfasl>>
genNfasl(size_t depth, size_t atomics, size_t states, size_t maxTrs) {
  return Catch2::GeneratorWrapper<Ptr<Nfasl>>
    (std::make_unique<NfaslGenerator>
     (depth, atomics, states, maxTrs));
}
#endif

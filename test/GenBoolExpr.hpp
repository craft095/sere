#ifndef GENBOOLEXPR_HPP
#define GENBOOLEXPR_HPP

#include "test/Tools.hpp"
#include "sat/Z3.hpp"
#include "ast/BoolExpr.hpp"

class BoolExprGenerator : public Catch2::IGenerator <Ptr<BoolExpr>> {
private:
  size_t maxDepth;
  size_t atoms;
  Ptr<BoolExpr> value;
public:
  BoolExprGenerator(size_t depth, size_t atoms_)
    : maxDepth(depth), atoms(atoms_) {
    next();
  }

  Ptr<BoolExpr> const& get() const override {
    return value;
  }

  bool next() override {
    value = make(maxDepth, atoms);
    return true;
  }

private:
  static size_t guessDepth(int d) {
    int d1 = Catch2::random(0, d).get();
    assert(d1 >= 0);
    assert(d1 <= d);
    return d1;
  }

  static Ptr<BoolExpr> makeTerm(size_t atoms) {
    static std::vector<Ptr<BoolExpr>> terms;
    static constexpr size_t maxAtoms = 128;
    assert(atoms < maxAtoms);
    if (terms.size() == 0) {
      terms.push_back(RE_TRUE);
      terms.push_back(RE_FALSE);
      for (size_t i = 0; i < maxAtoms; ++i) {
        terms.push_back(RE_VAR(i));
      }
    }
    size_t ix = choose((size_t)0, 2 + atoms - 1);
    return terms[ix];
  }

  static Ptr<BoolExpr> makeNot(size_t d, size_t atoms) {
    return RE_NOT(make(d - 1, atoms));
  }

  static Ptr<BoolExpr> makeAnd(size_t d, size_t atoms) {
    return RE_AND(make(d - 1, atoms), make(d - 1, atoms));
  }

public:
  static Ptr<BoolExpr> make(size_t depth, size_t atoms) {
    auto d = guessDepth(depth);
    if (d == 0) {
      return makeTerm(atoms);
    } else {
      auto u = [atoms, d]() { return makeNot(d, atoms); };
      auto v = [atoms, d]() { return makeAnd(d, atoms); };
      std::vector<std::function<Ptr<BoolExpr>()>> ops(2);
      ops[0] = u;
      ops[1] = v;
      return any_of(ops)();
    }
  }
};

inline Catch2::GeneratorWrapper<Ptr<BoolExpr>>
genBoolExpr(size_t depth, size_t atomics) {
  return Catch2::GeneratorWrapper<Ptr<BoolExpr>>
    (std::make_unique<BoolExprGenerator>
     (depth, atomics));
}

inline z3::expr makeZex(size_t depth, size_t atoms) {
  Ptr<BoolExpr> expr = BoolExprGenerator::make(depth, atoms);
  return boolSereToZex(*expr);
}

#endif // GENBOOLEXPR_HPP

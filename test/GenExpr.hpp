#ifndef GENEXPR_HPP
#define GENEXPR_HPP

#include "test/Tools.hpp"
#include "boolean/Expr.hpp"

class ExprGenerator : public Catch2::IGenerator <boolean::Expr> {
private:
  size_t maxDepth;
  size_t atoms;
  boolean::Expr value;
public:
  ExprGenerator(size_t depth, size_t atoms_)
    : maxDepth(depth), atoms(atoms_) {
    next();
  }

  boolean::Expr const& get() const override {
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

  static boolean::Expr makeTerm(size_t atoms) {
    static constexpr size_t maxAtoms = 128;
    static std::vector<boolean::Expr> terms;
    assert(atoms < maxAtoms);
    if (terms.size() == 0) {
      terms.push_back(boolean::Expr::value(true));
      terms.push_back(boolean::Expr::value(false));
      for (size_t i = 0; i < maxAtoms; ++i) {
        terms.push_back(boolean::Expr::var(i));
      }
    }
    size_t ix = choose((size_t)0, 2 + atoms - 1);
    return terms[ix];
  }

  static boolean::Expr makeNot(size_t d, size_t atoms) {
    return !make(d - 1, atoms);
  }

  static boolean::Expr makeAnd(size_t d, size_t atoms) {
    return make(d - 1, atoms) && make(d - 1, atoms);
  }

  static boolean::Expr makeOr(size_t d, size_t atoms) {
    return make(d - 1, atoms) || make(d - 1, atoms);
  }

public:
  static boolean::Expr make(size_t depth, size_t atoms) {
    auto d = guessDepth(depth);
    if (d == 0) {
      return makeTerm(atoms);
    } else {
      auto u = [atoms, d]() { return makeNot(d, atoms); };
      auto v = [atoms, d]() { return makeAnd(d, atoms); };
      std::vector<std::function<boolean::Expr()>> ops(2);
      ops[0] = u;
      ops[1] = v;
      return any_of(ops)();
    }
  }
};

inline Catch2::GeneratorWrapper<boolean::Expr>
genBoolExpr(size_t depth, size_t atomics) {
  return Catch2::GeneratorWrapper<boolean::Expr>
    (std::make_unique<ExprGenerator>
     (depth, atomics));
}

#endif // GENEXPR_HPP

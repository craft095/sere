#ifndef GENEXPR_HPP
#define GENEXPR_HPP

#include "test/Tools.hpp"
#include "BoolExpr.hpp"

class ExprGenerator : public Catch2::IGenerator <expr::Expr> {
private:
  size_t maxDepth;
  size_t atoms;
  expr::Expr value;
public:
  ExprGenerator(size_t depth, size_t atoms_)
    : maxDepth(depth), atoms(atoms_) {
    next();
  }

  expr::Expr const& get() const override {
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

  static expr::Expr makeTerm(size_t atoms) {
    static std::vector<expr::Expr> terms;
    if (terms.size() == 0) {
      terms.push_back(expr::Expr::value(true));
      terms.push_back(expr::Expr::value(false));
      for (size_t i = 0; i < maxAtoms; ++i) {
        terms.push_back(expr::Expr::var(i));
      }
    }
    size_t ix = choose((size_t)0, 2 + atoms - 1);
    return terms[ix];
  }

  static expr::Expr makeNot(size_t d, size_t atoms) {
    return !make(d - 1, atoms);
  }

  static expr::Expr makeAnd(size_t d, size_t atoms) {
    return make(d - 1, atoms) && make(d - 1, atoms);
  }

  static expr::Expr makeOr(size_t d, size_t atoms) {
    return make(d - 1, atoms) || make(d - 1, atoms);
  }

public:
  static expr::Expr make(size_t depth, size_t atoms) {
    auto d = guessDepth(depth);
    if (d == 0) {
      return makeTerm(atoms);
    } else {
      auto u = [atoms, d]() { return makeNot(d, atoms); };
      auto v = [atoms, d]() { return makeAnd(d, atoms); };
      std::vector<std::function<expr::Expr()>> ops(2);
      ops[0] = u;
      ops[1] = v;
      return any_of(ops)();
    }
  }
};

inline Catch2::GeneratorWrapper<expr::Expr>
genBoolExpr(size_t depth, size_t atomics) {
  return Catch2::GeneratorWrapper<expr::Expr>
    (std::make_unique<ExprGenerator>
     (depth, atomics));
}

#endif // GENEXPR_HPP

#include <iostream>
#include <memory>

#include "gtest/gtest.h"

#include "Language.hpp"
#include "EvalSere.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#define RE_LOC Located(Pos(__FILE__, __LINE__, 0))
#define RE_EMPTY std::make_shared<SereEmpty>(RE_LOC)
#define RE_TRUE std::make_shared<BoolValue>(RE_LOC, true)
#define RE_FALSE std::make_shared<BoolValue>(RE_LOC, false)
#define RE_VAR(n) std::make_shared<Variable>(RE_LOC, n)
#define RE_NOT(n) std::make_shared<BoolNot>(RE_LOC, n)
#define RE_AND(u,v) std::make_shared<BoolAnd>(RE_LOC, u ,v)
#define RE_INTERSECT(u,v) std::make_shared<Intersect>(RE_LOC, u,v)
#define RE_UNION(u,v) std::make_shared<Union>(RE_LOC, u,v)
#define RE_CONCAT(u,v) std::make_shared<Concat>(RE_LOC, u,v)
#define RE_STAR(u) std::make_shared<KleeneStar>(RE_LOC, u)

namespace Catch2 = Catch::Generators;

template <typename T>
T& any_of(std::vector<T>& fs) {
  size_t ix = Catch2::random(0, int(fs.size()) - 1).get();
  return fs[ix];
}

class LetterGenerator : public Catch2::IGenerator <Letter> {
private:
  size_t atoms;
  Letter value;
public:
  LetterGenerator(size_t atoms_) : atoms(atoms_), value("","") {
    next();
  }
  Letter const& get() const override {
    return value;
  }

  bool next() override {
    std::string p, n;

    for (size_t i = 0; i < atoms; ++i) {
      size_t c = Catch2::random(0, 1).get();
      if (c == 1) {
        p.push_back(char('a' + i));
      } else {
        n.push_back(char('a' + i));
      }
    }

    value = Letter(p,n);
    return true;
  }
};

Catch2::GeneratorWrapper<Letter> genLetter(size_t atoms) {
  return Catch2::GeneratorWrapper<Letter>(std::make_unique<LetterGenerator>(atoms));
}

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
    if (terms.size() == 0) {
      terms.push_back(RE_TRUE);
      terms.push_back(RE_FALSE);
      for (size_t i = 0; i < atoms; ++i) {
        char varName[] = { (char)('a' + i), 0 };
        terms.push_back(RE_VAR(varName));
      }
    }
    return any_of(terms);
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

static Catch2::GeneratorWrapper<Ptr<BoolExpr>> genBoolExpr(size_t depth, size_t atomics) {
  return Catch2::GeneratorWrapper<Ptr<BoolExpr>>(std::make_unique<BoolExprGenerator>(depth, atomics));
}


// std::shared_ptr<SereExpr> test0() {
//   return
//     RE_INTERSECT
//     (RE_STAR
//      (RE_UNION
//       (RE_EMPTY,
//        RE_TRUE)),
//      (RE_AND
//       (RE_FALSE,
//        (RE_NOT
//         (RE_VAR("x"))))));
// }

TEST(Sere, empty) {
  ASSERT_EQ(evalSere(*RE_EMPTY, {}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_EMPTY, {{"",""}}), Match_Failed);
}

TEST(Sere, boolean) {
  ASSERT_EQ(evalSere(*RE_TRUE, {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_FALSE, {{"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*RE_NOT(RE_TRUE), {{"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*RE_VAR("y"), {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_VAR("x"), {{"y", "x"}}), Match_Failed);

  SereChildPtr tc = RE_AND(RE_TRUE, RE_NOT(RE_VAR("x")));
  ASSERT_EQ(evalSere(*tc, {{"y", "x"}}), Match_Ok);
  ASSERT_EQ(evalSere(*tc, {{"x", "y"}}), Match_Failed);
  ASSERT_EQ(evalSere(*tc, {{"y", "x"}, {"y", "x"}}), Match_Failed);
  ASSERT_EQ(evalSere(*tc, {}), Match_Partial);
  // ASSERT_EQ(evalSere(*tc, {{"z", "y"}}), Match_Failed);
}

TEST(Sere, concat) {
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {}), Match_Partial);
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"x",""},{"y",""}}), Match_Ok);
  ASSERT_EQ(evalSere(*RE_CONCAT
                  (RE_VAR("x"),
                   RE_VAR("y")),
                  {{"xy",""},{"x","y"}}), Match_Failed);
  ASSERT_EQ(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"}}), Match_Partial);
  ASSERT_EQ(evalSere
         (*RE_CONCAT
          (RE_CONCAT
           (RE_VAR("x"),
            RE_VAR("y")
            ),
           RE_VAR("x")),
          {{"x","y"},{"x","y"}}), Match_Failed);
}

TEST_CASE("Bool to Z3") {
  constexpr size_t atoms = 3;
  auto expr = GENERATE(Catch2::take(10, genBoolExpr(3, atoms)));
  auto letter = GENERATE(Catch2::take(10, genLetter(atoms)));

  REQUIRE(evalBool(*expr, letter) == evalBoolZ3(*expr, letter));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  if (result != 0) {
    return result;
  }

  result = Catch::Session().run( argc, argv );

  return result;
}

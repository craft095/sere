#ifndef TESTTOOLS_HPP
#define TESTTOOLS_HPP

#include "catch2/catch.hpp"
#include <vector>

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

template <typename T>
std::vector<T> vector_of(size_t mn, size_t mx, std::function<T()> g) {
  size_t count = Catch2::random(mn, mx);
  std::vector<T> v;
  v.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    v.push_back(g());
  }
  return v;
}

#endif // TESTTOOLS_HPP

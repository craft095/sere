#ifndef TESTTOOLS_HPP
#define TESTTOOLS_HPP

#include "catch2/catch.hpp"
#include <vector>

#include "Letter.hpp"

namespace Catch2 = Catch::Generators;

template <typename T>
T choose(T mn, T mx) {
  return Catch2::random(mn, mx).get();
}

template <typename T>
T& any_of(std::vector<T>& fs) {
  size_t ix = choose((size_t)0, fs.size() - 1);
  return fs[ix];
}

template <typename T>
std::vector<T> vector_of(size_t mn, size_t mx, std::function<T()> g) {
  size_t count = choose(mn, mx);
  std::vector<T> v;
  v.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    v.push_back(g());
  }
  return v;
}

template <typename T>
std::set<T> set_of(size_t mn, size_t mx, std::function<T()> g) {
  size_t count = choose(mn, mx);
  std::set<T> v;
  for (size_t i = 0; i < count; ++i) {
    v.insert(g());
  }
  return v;
}

#endif // TESTTOOLS_HPP

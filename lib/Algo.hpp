#ifndef ALGO_HPP
#define ALGO_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <map>
#include <optional>

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
bool set_member (const std::set<T, CMP, ALLOC> &s, T v)
{
  return s.find(v) != s.end();
}

template <class K, class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
bool map_member (const std::map<K, T, CMP, ALLOC> &s, K k)
{
  return s.find(k) != s.end();
}

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<std::pair<T,T>, CMP, ALLOC> set_cross (
  const std::set<T, CMP, ALLOC> &s1, const std::set<T, CMP, ALLOC> &s2)
{
  std::set<std::pair<T,T>, CMP, ALLOC> s;
  for (auto q1 : s1) {
    for (auto q2 : s1) {
      s.insert(make_pair(q1, q2));
    }
  }
  return s;
}

template <class T, class U, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<U, CMP, ALLOC>
set_cross_with(
               const std::set<T, CMP, ALLOC> &s1,
               const std::set<T, CMP, ALLOC> &s2,
               std::function<U(T,T)> g)
{
  std::set<U, CMP, ALLOC> s;
  for (auto q1 : s1) {
    for (auto q2 : s2) {
      s.insert(g(q1, q2));
    }
  }
  return s;
}

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<T, CMP, ALLOC> set_difference (
  const std::set<T, CMP, ALLOC> &s1, const std::set<T, CMP, ALLOC> &s2)
{
  std::set<T, CMP, ALLOC> s;
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(s, s.begin()));
  return s;
}

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<T, CMP, ALLOC> set_intersects (
  const std::set<T, CMP, ALLOC> &s1, const std::set<T, CMP, ALLOC> &s2)
{
  std::set<T, CMP, ALLOC> s;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(s, s.begin()));
  return s;
}

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<T, CMP, ALLOC> set_unions (
                                   const std::set<T, CMP, ALLOC> &s1,
                                   const std::set<T, CMP, ALLOC> &s2)
{
  std::set<T, CMP, ALLOC> s;
  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(s, s.begin()));
  return s;
}

// TODO: very inefficient
template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
bool set_non_empty_intersection (
                                 const std::set<T, CMP, ALLOC> &s1,
                                 const std::set<T, CMP, ALLOC> &s2)
{
  return !set_intersects(s1, s2).empty();
}

template <typename K, typename T, typename ... Ts>
std::optional<T> get(const std::map<K,T,Ts...>& m, K k) {
  auto i = m.find(k);
  if (i == m.end()) {
    return std::nullopt;
  } else {
    return std::make_optional(i->second);
  }
}


#endif //ALGO_HPP

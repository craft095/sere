#ifndef ALGO_HPP
#define ALGO_HPP

#include <algorithm>
#include <iterator>
#include <set>

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
bool set_member (const std::set<T, CMP, ALLOC> &s, T v)
{
  return s.find(v) != s.end();
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
  return set_intersects(s1, s2).size() > 0;
}

#endif //ALGO_HPP

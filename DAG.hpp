#ifndef DAG_HPP
#define DAG_HPP

#include <set>
#include <map>

#include "Algo.hpp"

template <typename T>
struct DAG {
  struct Arc {
    T from;
    T to;
  };

  std::set<T> nodes;
  std::map<T, std::set<T>> arcs;

  /**
   * Find all nodes reachable from a set
   *
   */
  void reachableFrom(const std::set<T>& from, std::set<T>& qs) {
    std::set<T> candidates;
    std::set<T> newCandidates;
    qs = from;

    // fill candidates
    for (auto q : from) {
      auto next = get(arcs, q);
      if (next) {
        for (auto qn : *next) {
          if (!set_member(qs, qn)) {
            qs.insert(qn);
            candidates.insert(qn);
          }
        }
      }
    }

    while (!candidates.empty()) {
      std::set<T> candidates0;
      std::swap(candidates0, candidates);

      for (auto q : candidates0) {
        auto next = get(arcs, q);
        if (next) {
          for (auto qn : *next) {
            if (!set_member(qs, qn)) {
              qs.insert(qn);
              candidates.insert(qn);
            }
          }
        }
      }
    }
  }
};

#endif //DAG_HPP

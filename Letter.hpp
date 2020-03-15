#ifndef LETTER_HPP
#define LETTER_HPP

#include <set>
#include <vector>
#include <string>

#include "Language.hpp"

inline VarName make_varName(size_t ix) { return VarName {ix}; }
inline VarName make_varName(char x) { return VarName {size_t(x - 'a')}; }

class Letter {
public:
  std::set<VarName> pos;
  std::set<VarName> neg;

  Letter(const std::string& p, const std::string& n) {
    for (auto vp : p) pos.insert({size_t(vp - 'a')});
    for (auto vn : n) neg.insert({size_t(vn - 'a')});
  }

  std::string pretty() const;
};

typedef std::vector<Letter> Word;

extern std::string prettyWord(const Word& word);

#endif //LETTER_HPP

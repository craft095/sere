#ifndef LETTER_HPP
#define LETTER_HPP

#include <set>
#include <vector>
#include <string>

typedef std::string VarName;

class Letter {
public:
  std::set<VarName> pos;
  std::set<VarName> neg;

  Letter(const std::string& p, const std::string& n) {
    for (auto vp : p) pos.insert(VarName(&vp,1));
    for (auto vn : n) neg.insert(VarName(&vn,1));
  }

  std::string pretty() const;
};

typedef std::vector<Letter> Word;

extern std::string prettyWord(const Word& word);

#endif //LETTER_HPP

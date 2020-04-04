#ifndef LETTER_HPP
#define LETTER_HPP

#include <set>
#include <map>
#include <vector>
#include <string>

#include "Language.hpp"
#include "rt/RtPredicate.hpp"

typedef rt::Names Letter;

inline std::string prettyName(size_t k) {
  std::ostringstream str;
  str << 'x';
  str << k;
  return str.str();
}

inline rt::Names makeNames(const std::vector<size_t>& pos, const std::vector<size_t>& neg) {
  rt::Names names;
  names.resize(pos.size() + neg.size());
  for (auto vp : pos) names.set(vp, 1);
  for (auto vn : neg) names.set(vn, 0);
  return names;
}

inline std::string prettyNames(const rt::Names& names) {
  std::ostringstream str;

  for (size_t k = 0; k < names.size(); ++k) {
    if (!names.test(k)) {
      str << '!';
    }
    str << prettyName(k);
  }

  return str.str();
}

typedef std::vector<rt::Names> Word;

extern std::string prettyWord(const Word& word);

#endif //LETTER_HPP

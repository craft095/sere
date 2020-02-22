#include <sstream>
#include <string>

#include "Letter.hpp"

std::string Letter::pretty() const {
  std::ostringstream strm;
  strm << "[";

  for (auto& i : pos) {
    strm << i;
  }

  strm << "!";

  for (auto& i : neg) {
    strm << i;
  }

  strm << "]";
  return strm.str();
}

std::string prettyWord(const Word& word) {
  std::ostringstream strm;
  strm << "{";
  for (auto& l : word) strm << l.pretty();
  strm << "}";
  return strm.str();
}

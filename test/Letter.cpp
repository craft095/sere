#include <sstream>
#include <string>

#include "Letter.hpp"

std::string Letter::pretty() const {
  std::ostringstream strm;
  strm << "[";

  for (auto i : pos) {
    strm << i.pretty();
  }

  strm << "!";

  for (auto i : neg) {
    strm << i.pretty();
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

void letterToBitSet(const Letter& letter,
                    const std::map<VarName, rtp::Offset>& remap,
                    rtp::BitSet& bs) {
  for (auto n : letter.pos) {
    bs.set(remap.at(n), 1);
  }
  for (auto n : letter.neg) {
    bs.set(remap.at(n), 0);
  }
}

#include <sstream>
#include <string>

#include "test/Letter.hpp"

std::string prettyWord(const Word& word) {
  std::ostringstream strm;
  strm << "{";
  for (auto const& l : word) strm << prettyNames(l);
  strm << "}";
  return strm.str();
}

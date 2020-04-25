#ifndef AST_LOCATED_HPP
#define AST_LOCATED_HPP

#include <string>
#include <boost/format.hpp>

typedef std::string FileName;

struct Pos {
  FileName fileName;
  int line;
  int column;

  Pos(const FileName& fn, int ln, int cl)
    : fileName(fn), line(ln), column(cl) {}

  std::string prettyLineColumn() const {
    return (boost::format("%1%:%2%") % line % column).str();
  }

  std::string pretty() const {
    return (boost::format("%1%, %2%") % fileName % prettyLineColumn()).str();
  }

  bool operator== (const Pos& y) const {
    return
      fileName == y.fileName
      &&
      line == y.line
      &&
      column == y.column;
  }
};

class Located {
public:
  Located(const Pos& pos) : from(pos), to(pos) {}
  Located(const Pos& from_, const Pos& to_) : from(from_), to(to_) {}

  std::string pretty() const {
    if (from == to) {
      return (boost::format("[%s]") % from.pretty()).str();
    } else {
      return (boost::format("[%s-%s]") % from.pretty() % to.prettyLineColumn()).str();
    }
  }

private:
  Pos from;
  Pos to;
};

#define RE_LOC Located(Pos(__FILE__, __LINE__, 0))

#endif //AST_LOCATED_HPP

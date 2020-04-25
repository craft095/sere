#ifndef AST_COMMON_HPP
#define AST_COMMON_HPP

#include <string>
#include <sstream>
#include <memory>

#include "Located.hpp"

typedef std::string String;
struct VarName {
  size_t ix;

  std::string pretty() const {
    std::ostringstream stream;
    stream << 'x' << ix;
    return stream.str();
  }

  friend bool operator== (VarName x, VarName y) { return x.ix == y.ix; }
  friend bool operator< (VarName x, VarName y) { return x.ix < y.ix; }
};

inline VarName make_varName(size_t ix) { return VarName {ix}; }

template <typename T>
using Ptr = std::shared_ptr<T>;

class LocatedBase {
public:
  void setLoc(const Located& loc_) {
    loc = loc_;
  }

  const Located& getLoc() const {
    return loc;
  }

  LocatedBase(const Located& loc_) : loc(loc_) {}
  virtual const String pretty() const = 0;
  virtual ~LocatedBase() {}
private:
  Located loc;
};

#endif // AST_COMMON_HPP

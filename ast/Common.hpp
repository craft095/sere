#ifndef AST_COMMON_HPP
#define AST_COMMON_HPP

#include <string>
#include <memory>

#include "Located.hpp"

typedef std::string String;
struct VarName {
  static const char* names[26];

  size_t ix;

  const char* pretty() const {
    return names[ix];
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

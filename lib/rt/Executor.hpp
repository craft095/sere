#ifndef RTEXECUTOR_HPP
#define RTEXECUTOR_HPP

#include "rt/RtPredicate.hpp"
#include "Match.hpp"

namespace rt {

  class Executor {
  public:
    virtual Match getResult() const = 0;

    virtual void reset() = 0;
    virtual void advance(const Names& vars) = 0;

    virtual ~Executor() {}
  };

  class ExtendedExecutor {
  public:
    virtual const ExtendedMatch& getResult() const = 0;

    virtual void reset() = 0;
    virtual void advance(const Names& vars) = 0;

    virtual ~ExtendedExecutor() {}
  };

} // namespace rt

#endif //RTEXECUTOR_HPP

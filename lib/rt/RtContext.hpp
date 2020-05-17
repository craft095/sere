#ifndef RT_RTCONTEXT_HPP
#define RT_RTCONTEXT_HPP

namespace rt {

struct RtContext {
  static constexpr size_t NoValue = std::numeric_limits<size_t>::max();

  size_t longest;
  size_t shortest;

  static RtContext advance(RtContext ctx) {
    if (ctx.defined()) {
      ++ctx.longest;
      ++ctx.shortest;
    } else {
      ctx.longest = 0;
      ctx.shortest = 0;
    }
    return ctx;
  }

  RtContext() : longest(NoValue), shortest(NoValue) {}

  bool defined() const {
    return longest != NoValue && shortest != NoValue;
  }

  void started() {
    if (longest == NoValue) {
      longest = 0;
    }
    shortest = 0;
  }
  void merge(const RtContext& ctx) {
    if (defined() && ctx.defined()) {
      longest = std::max(longest, ctx.longest);
      shortest = std::min(shortest, ctx.shortest);
    } else if (ctx.defined()) {
      *this = ctx;
    }
  }
};

} // namespace rt

#endif // RT_RTCONTEXT_HPP

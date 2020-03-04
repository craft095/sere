#ifndef NFASL_HPP
#define NFASL_HPP

#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "c++/z3++.h"

using json = nlohmann::json;

namespace z3 {
  void to_json(json& j, const expr& e) {
    j = e.get_string();
  }
}

namespace nfasl {

  typedef z3::expr Predicate;
  typedef size_t State;
  typedef std::set<State> States;
  struct TransitionRule {
    Predicate phi;
    State state;
  };
  typedef std::vector<TransitionRule> Transitions;

  void to_json(json& j, const TransitionRule& p) {
    j = json{{"phi", p.phi}, {"state", p.state}};
  }

  class Nfasl {
  public:
    size_t atomicCount;
    size_t stateCount;
    State initial;
    States finals;
    std::vector<Transitions> transitions;
  };

  void to_json(json& j, const Nfasl& a) {
    j = json{
      {"atomicCount", a.atomicCount},
      {"stateCount",  a.stateCount},
      {"initial",     a.initial},
      {"finals",      a.finals},
      {"transitions", a.transitions}
    };
  }

  std::string pretty(const Nfasl& a) {
    std::ostringstream s;
    // serialization with pretty printing
    // pass in the amount of spaces to indent
    s << json(a).dump(4) << std::endl;
    return s.str();
  }

} // namespace nfasl

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}


int main( int argc, char* argv[] ) {
  // global setup...

  int result = Catch::Session().run( argc, argv );

  // global clean-up...

  return result;
}
#endif // NFASL_HPP

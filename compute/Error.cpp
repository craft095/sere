#include "Error.hpp"

#include <sstream>

namespace compute {

  Error::Error(const Located& loc_, const std::string& desc)
    : loc(loc_) {
    std::ostringstream stream{msg};
    stream << loc.pretty() << ": " << desc;
    msg = stream.str();
  }

  const char* Error::what() const throw() {
    return msg.c_str();
  }

} // namespace compute

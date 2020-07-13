#ifndef COMPUTE_ERROR_HPP
#define COMPUTE_ERROR_HPP

#include "ast/Located.hpp"

#include <string>

namespace compute {

  /**
   * Generic error
   */
  class Error : public std::exception {
  public:
    Error(const Located& loc, const std::string& msg = "");
    void setMessage(const std::string& msg);
    const char* what() const throw() override;
  private:
    Located loc;
    std::string msg;
  };

} // namespace compute

#endif // COMPUTE_ERROR_HPP

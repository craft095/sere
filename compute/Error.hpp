#ifndef COMPUTE_ERROR_HPP
#define COMPUTE_ERROR_HPP

#include "Typed.hpp"
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

  class ScalarTypeMismatch : public Error {
  public:
    ScalarTypeMismatch(const Located& loc,
                       const TypeIds& actual,
                       const TypeIds& expected);
  };

  class FuncTypeMismatch : public Error {
  public:
    FuncTypeMismatch(const Located& loc,
                     const FuncTypes& actual,
                     const FuncTypes& expected);
  };

  class BadApplication : public Error {
  public:
    BadApplication(const Located& loc,
                   Func::Ptr func,
                   TypedNodes& args);
  };

  class NameNotFound : public Error {
  public:
    NameNotFound(const Located& loc, const Ident::Name& name);
  };

  class ScalarExpected : public Error {
  public:
    ScalarExpected(const Located& loc, const Ident::Name& name);
  };

  class FuncExpected : public Error {
  public:
    FuncExpected(const Located& loc, const Ident::Name& name);
  };

} // namespace compute

#endif // COMPUTE_ERROR_HPP

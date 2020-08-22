#include "Error.hpp"

#include <nlohmann/json.hpp>
#include <sstream>

namespace compute {

  Error::Error(const Located& loc_, const std::string& desc)
    : loc(loc_) {
    std::ostringstream stream{msg};
    stream << loc.pretty() << ": " << desc;
    msg = stream.str();
  }

  void Error::setMessage(const std::string& msg) {
    this->msg = msg;
  }

  const char* Error::what() const throw() {
    return msg.c_str();
  }

  ScalarTypeMismatch::ScalarTypeMismatch(const Located& loc,
                                         const TypeIds& actual,
                                         const TypeIds& expected)
    : Error(loc) {
    std::ostringstream stream;
    stream << "type mismatch, expected: "
           << pretty(expected)
           << "actual: "
           << pretty(actual);

    setMessage(stream.str());
  }

  FuncTypeMismatch::FuncTypeMismatch(const Located& loc,
                                     const FuncTypes& actual,
                                     const FuncTypes& expected)
    : Error(loc) {
    std::ostringstream stream;
    stream << "function type mismatch, expected: "
           << pretty(expected)
           << "actual: "
           << pretty(actual);

    setMessage(stream.str());
  }

  BadApplication::BadApplication(const Located& loc,
                                 Func::Ptr /*func*/,
                                 TypedNodes& /*args*/)
    : Error(loc) {
    std::ostringstream stream;
    stream << "function can not be applied to its args";

    setMessage(stream.str());
  }

  NameNotFound::NameNotFound(const Located& loc,
                                         const Ident::Name& name)
    : Error(loc) {
    std::ostringstream stream;
    stream << "unknown name `" << name << "'";

    setMessage(stream.str());
  }

  ScalarExpected::ScalarExpected(const Located& loc,
                                 const Ident::Name& name)
    : Error(loc) {
    std::ostringstream stream;
    stream << "scalar value expected, but name `" << name << "' denotes a function";

    setMessage(stream.str());
  }

  FuncExpected::FuncExpected(const Located& loc,
                             const Ident::Name& name)
    : Error(loc) {
    std::ostringstream stream;
    stream << "function expected, but name `" << name << "' denotes a scalar value";

    setMessage(stream.str());
  }

} // namespace compute

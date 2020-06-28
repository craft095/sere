#ifndef COMPUTE_PARSER_HPP
#define COMPUTE_PARSER_HPP

#include <iostream>
#include "Ast.hpp"

namespace compute {

  /**
   * Parse error
   */
  class ParseError : public std::exception {
  public:
    ParseError(const Located& loc, const std::string& msg);
    const char* what() const throw() override;
  private:
    Located loc;
    std::string msg;
  };

  /**
   * Parse input stream into AST
   *
   * @param [in] file input stream name
   * @param [in] stream input stream
   * @return AST root pointer
   * @throws parser::ParseError if parse/scan errors occur
   */
  extern Root::Ptr parse(const std::string& file, std::istream& stream);

  /**
   * Parse input stream into AST
   *
   * @param [in] stream input stream
   * @return AST root pointer
   * @throws parser::ParseError if parse/scan errors occur
   */
  extern Root::Ptr parse(std::istream& stream);
} // namespace compute

#endif // COMPUTE_PARSER_HPP

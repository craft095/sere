#ifndef PARSER_HPP
#define PARSER_HPP

#include <map>
#include <iostream>
#include "ast/SereExpr.hpp"

namespace parser {
  /**
   * Parse results
   */
  struct ParseResult {
    Ptr<SereExpr> expr;
    std::map<std::string, size_t> vars;
  };

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
   * Parse input stream into complete SERE
   *
   * @param [in] file input stream name
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws parser::ParseError if parse/scan errors occur
   */
  extern ParseResult parse(const std::string& file, std::istream& stream);

  /**
   * Parse input stream into complete SERE
   *
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws parser::ParseError if parse/scan errors occur
   */
  extern ParseResult parse(std::istream& stream);
} // namespace parser

#endif // PARSER_HPP

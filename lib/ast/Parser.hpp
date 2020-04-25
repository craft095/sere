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
   * Parse input stream into complete SERE
   *
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws std::invalid_argument if parse/scan errors occur
   */
  extern ParseResult parse(std::istream& stream);
} // namespace parser

#endif // PARSER_HPP

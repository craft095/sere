#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include "Language.hpp"

namespace parser {
  /**
   * Parse input stream into complete SERE
   *
   * @param [in] stream input stream
   * @return SERE AST root pointer
   * @throws std::invalid_argument if parse/scan errors occur
   */
  extern Ptr<SereExpr> parse(std::istream& stream);
} // namespace parser

#endif // PARSER_HPP

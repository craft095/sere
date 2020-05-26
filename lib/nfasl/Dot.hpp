#ifndef NFASL_DOT_HPP
#define NFASL_DOT_HPP

#include "nfasl/Nfasl.hpp"

namespace nfasl {

/**
 * Dump NFALS into Graphviz DOT-formatted file
 */
extern void toDot(const Nfasl& a, const std::string& file);

}

#endif // NFASL_DOT_HPP

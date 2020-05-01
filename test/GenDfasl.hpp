#ifndef GENDFASL_HPP
#define GENDFASL_HPP

#include "nfasl/Dfasl.hpp"
#include "catch2/catch.hpp"

extern Ptr<dfasl::Dfasl>
makeDfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs);

extern Catch2::GeneratorWrapper<Ptr<dfasl::Dfasl>>
genDfasl(size_t depth, size_t atomics, size_t states, size_t maxTrs);

#endif // GENDFASL_HPP

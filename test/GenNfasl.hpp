#ifndef GENNFASL_HPP
#define GENNFASL_HPP

#include "nfasl/Nfasl.hpp"
#include "catch2/catch.hpp"

extern Ptr<nfasl::Nfasl>
makeNfasl(size_t depth, size_t atoms, size_t states, size_t maxTrs);

extern Catch2::GeneratorWrapper<Ptr<nfasl::Nfasl>>
genNfasl(size_t depth, size_t atomics, size_t states, size_t maxTrs);

#endif // GENNFASL_HPP

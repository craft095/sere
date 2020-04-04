#ifndef EVALNFASL_HPP
#define EVALNFASL_HPP

#include "Match.hpp"
#include "Nfasl.hpp"
#include "Letter.hpp"

extern Match evalNfasl(const nfasl::Nfasl& a, const Word& word);

#endif // EVALNFASL_HPP

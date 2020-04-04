#ifndef EVALRTNFASL_HPP
#define EVALRTNFASL_HPP

#include "test/Letter.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

extern Match evalRtNfasl(const rt::Nfasl& nfasl, const Word& word);

#endif // EVALRTNFASL_HPP

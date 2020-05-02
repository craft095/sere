#ifndef EVALRT_HPP
#define EVALRT_HPP

#include "test/Letter.hpp"
#include "rt/RtDfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

extern Match evalRtDfasl(const rt::Dfasl& nfasl, const Word& word);
extern Match evalRtNfasl(const rt::Nfasl& nfasl, const Word& word);

#endif // EVALRT_HPP

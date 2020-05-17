#ifndef EVALRT_HPP
#define EVALRT_HPP

#include "test/Letter.hpp"
#include "rt/RtDfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

extern Match evalRtDfasl(const rt::Dfasl& dfasl, const Word& word);
extern Match evalRtNfasl(const rt::Nfasl& nfasl, const Word& word);
extern ExtendedMatch evalExtendedRtNfasl(const rt::Nfasl& nfasl, const Word& word);
extern Match evalRt(rt::ExecutorPtr executor, const Word& word);

#endif // EVALRT_HPP

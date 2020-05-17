#include "test/Letter.hpp"
#include "rt/RtDfasl.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

Match evalRtNfasl(const rt::Nfasl& nfasl, const Word& word) {
  rt::NfaslContext context{std::make_shared<rt::Nfasl>(nfasl)};
  for (auto& letter : word) {
    context.advance(letter);
  }
  return context.getResult();
}

Match evalRtDfasl(const rt::Dfasl& dfasl, const Word& word) {
  rt::DfaslContext context{std::make_shared<rt::Dfasl>(dfasl)};
  for (auto& letter : word) {
    context.advance(letter);
  }
  return context.getResult();
}

ExtendedMatch evalExtendedRtNfasl(const rt::Nfasl& nfasl, const Word& word) {
  rt::NfaslExtendedContext context{std::make_shared<rt::Nfasl>(nfasl)};
  for (auto& letter : word) {
    context.advance(letter);
  }
  return context.getResult();
}

Match evalRt(rt::ExecutorPtr executor, const Word& word) {
  for (auto& letter : word) {
    executor->advance(letter);
  }
  return executor->getResult();
}

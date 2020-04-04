#include "test/Letter.hpp"
#include "rt/RtNfasl.hpp"
#include "Match.hpp"

Match evalRtNfasl(const rt::Nfasl& nfasl, const Word& word) {
  rt::NfaslContext context{nfasl};
  for (auto& letter : word) {
    context.advance(letter);
  }
  return context.getResult();
}

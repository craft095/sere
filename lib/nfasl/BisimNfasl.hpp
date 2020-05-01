#ifndef BISIMNFASL_HPP
#define BISIMNFASL_HPP

namespace nfasl {
  class Nfasl;
  extern void clean(const Nfasl& nfasl, Nfasl& cleaned);
  extern void minimize(const Nfasl& a, Nfasl& b);
  extern void complement(const nfasl::Nfasl& a, nfasl::Nfasl& b);
}

#endif //BISIMNFASL_HPP

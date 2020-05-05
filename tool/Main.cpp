#include "api/sere.hpp"

#include "api/sere.hpp"

#include <string>
#include <iostream>

int main(int, char*[]) {
  //const char expr[] = "(A ; B[*] ; ~(C | D) ; E) & F[+]";
  const char expr[] = "(A ; B[*]) & F[+]";

  struct sere_options opts
    = {
       SERE_TARGET_DFASL,
       SERE_FORMAT_JSON,
       0,
       0 };

  struct sere_compiled compiled;
  int r = sere_compile(expr, &opts, &compiled);

  if (r != 0) {
    sere_release(&compiled);
    return r;
  }

  std::cout << compiled.content << std::endl;

  sere_release(&compiled);
  return 0;
}

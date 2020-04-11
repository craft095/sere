# T

The purpose of the library is to promote runtime verification
of temporal properties, specified with [sequential extended
regular expressions]().

Sequential extended regular expressions or SEREs are very
concise, flexible way to specify temporal properites.

SERE can describe any regular language (without infinite words).
Semantics:

- $\phi$


SERE is very seimilar to normal reqular expressions, which
are especially often used with texts. As a normal RE, SERE
comprise concatenation, union and kleene star (`*`) operators.
But where REs use symbols from some alphabet (like characters
in text), SEREs use arbitrary boolean predicates over set
boolean variables.

It is al about succinctness: in general SERE with k boolean
variables are equivalent to RE over alphbet of size 2^k.
This explains why some properities, expressible in SERE,
are practically inexpressible in RE.

The library:

- parsing from SERE syntax into AST SereExpr (Language.hpp)
- translation from SereExpr into Non-deterministic automaton with symbolic lables (NFASL)
- clean/normalize/minimize NFASL (BisimNfasl.hpp)
- translate NFASL into runtime NFASL (rt/RtNfasl.hpp)
- evaluate RtNfasl over stream of events in real time

In the future, there will be added:
- translation from non-deterministic to deterministic automaton
- reverse mode: generating sequence of event, matching the given expression


SERE

Being very similar to well known reqular expressions, t

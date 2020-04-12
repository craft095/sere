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

## Docs

`()` - matches empty sequence
`p` - matches sequence of size 1 where the only element satisfies
      boolean  predicate `p`
`u ; v` - if `u` matches `word0` and `v` matches `word1`
          then `u ; v` matches their concatenation `word0 + word1`
`u : v` - if `u` matches `[wu_0..wu_(n-1),X]`
          and `v` matches `[X,wv1..wvn]`
          then `u : v` matches their fusion `[wu_0..wu_(n-1),X,wv1..wvn]`
`u & v` - intersection of languages defined by `u` and `v`
`u | v` - union of languages defined by `u` and `v`
`u[*]` - if `u` matches word `w` then `u[*]` matches zero or more
         repeation on `w`: [], w, w+w, w+w+w, etc
`u[+]` - if `u` matches word `w` then `u[*]` matches one or more
         repeation on `w`: w, w+w, w+w+w, etc
`PERMUTE(u0,..,un)` - union of all possible concatenations of `u0..un`
         E.g. `PERMUTE(a,b) = a;b | b;a`
`ABORT(u,e)` - preempt any word `w` partially (or fully) matched
               by `u` by any suffix `s` matched by `e`
`u{n}`   - repeat exactly n times any word matched by `u`.
           `u{n} = u ; ..n times.. ; u`
`u{n,}`  - repeat at least n times any word matched by `u`.
           `u{n} = u ; ..n times.. ; u ; (u[*])`
`u{n,m}` - repeat at least n and at most m times any word matched by `u`.

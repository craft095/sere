# Why has this library been created?

The purpose of the library is to promote runtime verification
of temporal properties, specified with [sequential extended
regular expressions](http://www.sergiomover.eu/paper/ciaa10.pdf).

The other (and perhaps the most important) reason - this
is just an adventure into the world of fascinating algorithms:
partition refinement, SAT solvers, predicate simplification etc.

## What is SERE?

Sequential extended regular expressions or SEREs are very
concise, flexible way to specify temporal properites.

SERE can describe any regular language (without infinite words).
Semantics: TBD

SERE is very seimilar to normal reqular expressions, which
are especially often used with texts. As a normal RE, SERE
comprise concatenation, union and kleene star (`*`) operators.
But where REs use symbols from some alphabet (like characters
in text), SEREs use arbitrary boolean predicates over set
boolean variables.

It is all about succinctness: in general SERE with `k` boolean
variables are equivalent to RE over alphabet of size `2^k`.
This explains why some properties, expressible in SERE,
are practically inexpressible in RE.

The library:

- parsing from SERE syntax into AST SereExpr
- translate from SereExpr into Non-deterministic automaton with symbolic lables (NFASL)
- clean/normalize/minimize NFASL (BisimNfasl.hpp)
- translate NFASL into runtime NFASL (rt/RtNfasl.hpp)
- evaluate RtNfasl over a stream of events in real time
- translate from non-deterministic to deterministic automaton (DFASL)
- translate DFASL into runtime DFASL (rt/RtDfasl.hpp)

There is also a python3 bindings, which provides a simple way
to try the SERE.

In the future, there will be added:
- reverse mode: generating sequence of event, matching the given expression

## Syntax

* `()` - matches empty sequence
* `p` - matches sequence of size 1 where the only element satisfies
      boolean  predicate `p`
* `u ; v` - if `u` matches `word0` and `v` matches `word1`
          then `u ; v` matches their concatenation `word0 + word1`
          (number of states is a sum)
* `u : v` - if `u` matches `[wu_0..wu_(n-1),X]`
          and `v` matches `[X,wv1..wvn]`
          then `u : v` matches their fusion `[wu_0..wu_(n-1),X,wv1..wvn]`
          (number of states is a sum)
* `u & v` - intersection of languages defined by `u` and `v`
          (number of states is a multiplication)
* `u | v` - union of languages defined by `u` and `v`
          (number of states is a sum)
* `u[*]` - if `u` matches word `w` then `u[*]` matches zero or more
         repeation on `w`: [], w, w+w, w+w+w, etc
* `u[+]` - if `u` matches word `w` then `u[*]` matches one or more
         repeation on `w`: w, w+w, w+w+w, etc
* `PERMUTE(u0,..,un)` - union of all possible concatenations of `u0..un`
         E.g. `PERMUTE(a,b) = a;b | b;a`
         (number of states is `n!*(|u0|+...+|un|)`)
* `ABORT(u,e)` - preempt any word `w` partially (or fully) matched
               by `u` by any suffix `s` matched by `e`
* `~u` - complement L(u) (you can think of it like a negation).
         This operation requires determinisation and may result in
         exponential explosion of state number
* `u{n}` - repeat exactly n times any word matched by `u`.
           `u{n} = u ; ..n times.. ; u`
* `u{n,}` - repeat at least n times any word matched by `u`.
           `u{n} = u ; ..n times.. ; u ; (u[*])`
* `u{n,m}` - repeat at least n and at most m times any word matched by `u`.

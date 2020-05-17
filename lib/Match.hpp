#ifndef MATCH_HPP
#define MATCH_HPP

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdint.h>
#endif

enum Match {
  Match_Ok,
  Match_Partial,
  Match_Failed,
};

/**
 * The result of a SERE evaluation. There are three
 * possible outcomes: ok, partial & failed
 * Extended means that it also provide information about
 * which event subsequence matches the SERE (it is always a suffix of a word/stream)
 *
 * `failed` is flagged with Match_Failed (field `match`)
 * and is not supplied with additional information
 * As we find a matching stream suffix, `failed` can only be
 * returned if SERE is equivalent to FALSE: i.e. there is no
 * seqence matching the SERE (even potentially).
 *
 * `partial` is flagged with Match_Partial and
 * there is a `paritial.horizon` - length of a longest stream suffix
 * that (potentially) starts a matching subsequence.
 * Client code may use this information, for example, to discard
 * any events ealier then `paritial.horizon`
 *
 * `ok` is flagged with Match_Ok and it comes with the following
 * information:
 * `ok.horizon` - the same meaning as in `partial.horizon`
 * `ok.longest` - length of a longest match
 * `ok.shortest` - length of a shortest match
 *
 * Note, that longest/shortest are with regard to current stream
 * state. Streams are potentially infinite, but SERE library is only
 * dealing with finite stream prefix from start to stream head.
 *
 * Example:
 *   ((B;A) | (B;B;B)[*] | (B[*] ; A ; A))
 *
 *   stream: [] --> partial (horizon = 0)
 *   stream: [B] --> partial (horizon = 1)
 *   stream: [B,B] --> partial (horizon = 2)
 *   stream: [B,B,B] --> ok (longest=3, shortest=3, horizon = 3)
 *   stream: [B,B,B,A] --> ok (longest=2, shortest=2, horizon = 4)
 *   stream: [B,B,B,A,C] --> partial (horizon = 0)
 *   stream: [B,B,B,A,C,B] --> partial (horizon = 1)
 */
struct ExtendedMatch {
  enum Match match;
  union {
    struct {
      size_t longest; /** the ealiest event starting a matched subsequence */
      size_t shortest; /** the latest event starting a matched subsequence */
      size_t horizon; /** the ealiest event to find a match */
    } ok;
    struct {
      size_t horizon; /** the ealiest event to find a match */
    } partial;
  };
};


#endif // MATCH_HPP

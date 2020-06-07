import sere

# This is very basic example, which can easily canbe handled
# with normal reegular expressions.
# It helps to understand how to uses the library

messages = [  "start_session"
            , "list_items"
            , "get_item_info"
            , "get_item_info"
            , "close_session"
            , "start_session"
            , "get_item_info"
            , "place_order"
            , "close_session"
            , "start_session"
            , "account_settings"
            , "list_items"
            , "get_item_info"
            , "place_order"
            , "get_item_info"
            , "list_items"
            , "get_item_info"
            , "place_order"
            , "close_session"
            , "start_session"
            , "close_session"
            , "start_session"
            ]

#
# Task: find completed sessions, resulted in a purchase
#
# Completed sessions start with `start_session` and end with `close_session`
# Event `place_order` means puchase
#
# Note: we have to identify entire session (for futher analysis, for example),
# so we use `extended` version, which not only detects patterns but also
# is able to say where matching subsequence starts.
# Simple (not extended) API is more efficient but is only able to whether given
# pattern matches the stream or not.
#

# Let's first start building expression in an incremental way
# 1) define expression to describe any completed session with some content:
def session(content):
    return "started ; ((!started && !closed)[*] & (%s)) ; closed" % content

# Session starts with `started` and ends with `closed` predicated. We use concatenation operator `;` to
# say that right hand side follows immediately after left hand side. The most iteresting stuff is
# in between, let's dissect it:
# (
#  !closed && !started --- prohibit started/closed predicates to become `true`
# )[*]                 --- [*] means the same as `*` in normal regular expression: zero or more times
#                      --- altogether it means any sequence which contains
#                      --- neither `started` nor `closed`
# &                    --- `&` has meaning of intersection of languages on both sides
# content              --- some expression for session content (see below)

# 2) now, we want only sessions with at least one purchase:
def purchase():
    return session("true[*] ; purchase ; true[*]")
# The only new thing here is `true`, which means `any predicate` and it is analogous to `.` (any symbol)
# in normal regular expressions. So this means: any sequence with at least one `purchase` triggerred inside.

expression = purchase()

# Compile the expression into NFASL (for a while, it is the only supported target for extended processing).
compiled = sere.compile(expression, "nfasl")

# `compiled.content()` is just a plain JSON file with NFASL inside - you can serialize it or use immediately.

# Load serialized NFASL
rt = sere.load_extended(compiled.content())

# Start stream processing (in our case just walking through the list of log entries)
# Remeber that in our expression all names are actually predicates
# It is worth to note what are predicates? Well, in our simple case, we can say that
# - predicate `started` becomes `True` if we see message `start_session`
# - predicate `closed` becomes `True` if we see message `close_session`
# - predicate `purchase` becomes `True` if we see message `place_order`
# Other messages can be safely ignored.

# In this variable we will keep suffix of a stream (remember that our task
# is to find sequence of log messages of a completed session with a purchase)
suffix = []

for message in messages:
    # remeber that in our expression all names are actually predicates
    # so, `rt.match` accept a dictionary where every predicate mentioned in the expression
    # is either `True` or `False`
    predicates = {}
    predicates['started'] = message == 'start_session'
    predicates['closed'] = message == 'close_session'
    predicates['purchase'] = message == 'place_order'

    suffix.append(message)
    rt.feed(predicates)
    r = rt.matched()

    # `rt.matched()` returns object with fields:
    # - `match`
    #  - `0` - suffix of a stream matched expression
    #  - `1` - suffix of a stream partially matched expression
    #  - `2` - failed to find any match
    # - `horizon` - the maximal length of stream suffix which has a potential
    #               to match the expression
    # - `shortest` - (only valid for `match == 0`) length of a matching
    #                minimal stream suffix
    # - `longest` - (only valid for `match == 0`) length of a matching
    #               maximal stream suffix
    #
    # Note `horizon` - this parameter allows us to know how many elements
    # of the stream we have to keep in memory. It is important for
    # stream processing.
    # In our simple case, shortest will always be equal to longest.

    if r.match == 2: # match failed
        suffix = []
    elif r.match == 1: # partial match
        suffix = suffix[-r.horizon:]
    else:
        # we found the match
        print('found: ', suffix[-r.shortest:])
        suffix = []

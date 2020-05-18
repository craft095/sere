import sere

result = False

try:
    sere.compile("a;;", "dfasl")
except ValueError:
    result = True

x = sere.compile("a;b", "dfasl")
y = sere.load(x.content())

events = [{ 'a' : True, 'b' : False },
          { 'a' : True, 'b' : True } ]

r = y.match(events)

result = result and r == 0

u = sere.compile("a;b", "nfasl")
v = sere.load_extended(x.content())

events = [{ 'a' : True, 'b' : False },
          { 'a' : True, 'b' : True },
          { 'a' : True, 'b' : True } ]

r = v.match(events)

result = result and r.match == 0 \
                and r.shortest == 2 \
                and r.longest == 2 \
                and r.horizon == 2

if result:
    exit(0)
else:
    exit(1)

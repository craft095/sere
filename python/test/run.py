import sere

x = sere.compile("a;b", "dfasl")
y = sere.load(x.content())

events = [{ 'a' : True, 'b' : False },
          { 'a' : True, 'b' : True } ]

r = y.match(events)

print('Result:', r)

if r == 0:
    exit(0)
else:
    exit(1)

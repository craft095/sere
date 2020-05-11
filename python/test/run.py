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

if result:
    exit(0)
else:
    exit(1)

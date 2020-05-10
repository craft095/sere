import sere

x = sere.compile("a;b", "dfasl")
y = sere.load(x.content())
print('atomics:', y.atomic_count())
remap = dict([(str(y.atomic_name(i), 'utf-8'),i) for i in range(0,y.atomic_count())])

print(remap)

events = [{ 'a' : True, 'b' : False },
          { 'a' : True, 'b' : True } ]

def pack_event(event):
    l = [(remap.get(k), 1 if v else 0)
         for k,v
         in event.items()]
    l.sort()
    [_,vs] = zip(*l)
    return bytes(vs)

print('r:', y.get_result())
for e in events:
    y.advance(pack_event(e))
    print('r:', y.get_result())

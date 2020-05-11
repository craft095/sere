import sys
import logging
import serec

__author__    = 'Dmitry Kulagin'
__copyright__ = 'Copyright (C) 2020 Dmitry Kulagin'
__license__   = 'MIT'
__version__   = '0.1'

logging.basicConfig(stream=sys.stdout, level=logging.INFO)

def compile(expr, target = 'dfasl'):
    return serec.compile(expr, target)

def load(content):
    return Sere(serec.load(content))

class Sere:
    def __init__(self, ctx):
        self.ctx = ctx
        self.remap = dict(
            [(str(ctx.atomic_name(i), 'utf-8'),i)
             for i in range(0,ctx.atomic_count())])

    def _pack_event(self, event):
        l = [(self.remap.get(k), 1 if v else 0)
            for k,v
            in event.items()]
        l.sort()
        [_,vs] = zip(*l)
        return bytes(vs)

    def reset(self):
        self.ctx.reset()

    def matched(self):
        return self.ctx.get_result()

    def feed(self, event):
        self.ctx.advance(self._pack_event(event))

    def match(self, events):
        self.reset()
        for e in events:
            self.feed(e);
        return self.matched()

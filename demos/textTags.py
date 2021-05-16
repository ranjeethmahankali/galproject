import pygalfunc as pgf
import pygalview as pgv

POINTS = [
    [0, 0, 0],
    [1, 0, 0],
    [1, 1, 0],
    [-.3, 1, 0],
    [0, -1, 0],
]

# WORDS = [str(i) for i in range(len(POINTS))]
WORDS = ["apples", "bananas", "figs", "kiwis", "peaches"]

pts, = pgf.listvec3(POINTS)
words, = pgf.liststring(WORDS)

pgv.tags("indices", pts, words)

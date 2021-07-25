import pygalfunc as pgf
import pygalview as pgv

POINTS = [
    [0, 0, 0],
    [1, 0, 0],
    [1, 1, 0],
    [-.3, 1, 0],
    [0, -1, 0],
]

WORDS = [str(i) for i in range(len(POINTS))]

pts = pgf.listvec3(POINTS)
words = pgf.liststring(WORDS)
cloud = pgf.pointCloud3d(pts)
idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
circ, *_ = pgf.boundingCircle(cloud)

pgv.tags("indices", pts, words)
pgv.show("circle", circ)
pgv.show("points", cloud)
pgv.print("Point at index", idxPt)

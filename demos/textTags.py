import pygalfunc as pgf
import pygalview as pgv

POINTS = [
    (0, 0, 0),
    (1, 0, 0),
    (1, 1, 0),
    (-.3, 1, 0),
    (0, -1, 0),
]

pts = pgf.var_vec3(POINTS)
nums = pgf.toString(pgf.series(pgf.var_int(0), pgf.var_int(1), pgf.listLength(pts)))
idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
circ, *_ = pgf.boundingCircle(pts)

pgv.show("indices", pgv.tags(nums, pts))
pgv.show("circle", circ)
pgv.show("points", pts)
pgv.print("Point at index", idxPt)

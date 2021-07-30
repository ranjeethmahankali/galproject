import pygalfunc as pgf
import pygalview as pgv
import os

POINTS = [
    [0, 0, 0],
    [1, 0, 0],
    [1, 1, 0],
    [-.3, 1, 0],
    [0, -1, 0],
]

gpath = os.path.abspath(os.path.join(os.path.dirname(
    os.path.abspath(__file__)), "../assets/transmitterGlyph.png"))

glyphs = [("transmitter", gpath) for _ in range(len(POINTS))]
pgv.loadGlyphs(glyphs)
glyphIndices = [pgv.glyphIndex(g[0]) for g in glyphs]

pts = pgf.listvec3(POINTS)
glyphIndices = pgf.listi32(glyphIndices)
cloud = pgf.pointCloud3d(pts)
idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
circ, *_ = pgf.boundingCircle(cloud)

pgv.glyphs("indices", pts, glyphIndices)
pgv.show("circle", circ)
pgv.show("points", cloud)
pgv.print("Point at index", idxPt)

import pygalfunc as pgf
import pygalview as pgv
import os

POINTS = [
    (0, 0, 0),
    (1, 0, 0),
    (1, 1, 0),
    (-.3, 1, 0),
    (0, -1, 0),
]

GLYPHDATA = [("receiver", "/home/rnjth94/works/YouTube/GAL_BoundingCircle/receiverDishGlyph.png"),
              ("transmitter", "/home/rnjth94/works/YouTube/GAL_BoundingCircle/transmitterGlyph.png")]

GLYPHINDICES = {"receiver": 0, "transmitter": 0}


def initGlyphs():
    global GTRANSMITTER, GRECEIVER
    indices = pgv.loadGlyphs(GLYPHDATA)
    GLYPHINDICES["receiver"] = indices[0]
    GLYPHINDICES["transmitter"] = indices[1]


if __name__ == "__main__":
    initGlyphs()
    pts = pgf.var_vec3(POINTS)
    cloudGlyphs = pgf.var_int([GLYPHINDICES["receiver"] for _ in range(len(POINTS))])
    idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
    circ, center, radius = pgf.boundingCircle(pts)
    center3 = pgf.vec3FromVec2(center)
    centerGlyph = pgf.var_int(GLYPHINDICES["transmitter"])

    pgv.show("glyph1", pgv.glyphs(cloudGlyphs, pts))
    pgv.show("glyph2", pgv.glyphs(centerGlyph, center3))
    pgv.show("circle", circ)
    pgv.show("points", pts)
    pgv.show("center", center3)
    pgv.print("Point at index", idxPt)

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

GLYPHDATA = ["/home/rnjth94/works/YouTube/GAL_BoundingCircle/receiverDishGlyph.png",
             "/home/rnjth94/works/YouTube/GAL_BoundingCircle/transmitterGlyph.png"]


def initGlyphs():
    global GTRANSMITTER, GRECEIVER
    return pgv.loadGlyphs(GLYPHDATA)


if __name__ == "__main__":
    glyphs = initGlyphs()
    pts = pgf.var_vec3(POINTS)
    cloudGlyphs = pgf.var_int([glyphs[0]
                              for _ in range(len(POINTS))])
    idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
    circ, center, radius = pgf.boundingCircle(pts)
    center3 = pgf.vec3FromVec2(center)
    centerGlyph = pgf.var_int(glyphs[1])

    pgv.show("glyph1", pgv.glyphs(cloudGlyphs, pts))
    pgv.show("glyph2", pgv.glyphs(centerGlyph, center3))
    pgv.show("circle", circ)
    pgv.show("points", pts)
    pgv.show("center", center3)
    pgv.print("Point at index", idxPt)

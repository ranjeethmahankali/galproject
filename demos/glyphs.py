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

GTRANSMITTER = 0
GRECEIVER = 0


def initGlyphs():
    global GTRANSMITTER, GRECEIVER
    relpaths = ["../assets/transmitterGlyph.png", "../assets/receiverDishGlyph.png"]
    gpaths = [os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), p)) for p in relpaths]
    gnames = ["transmitter", "receiver"]
    glyphs = [(gname, gpath) for gname, gpath in zip(gnames, gpaths)]
    print(glyphs)
    pgv.loadGlyphs(glyphs)
    GTRANSMITTER = pgv.glyphIndex(gnames[0])
    GRECEIVER = pgv.glyphIndex(gnames[1])


if __name__=="__main__":
    initGlyphs()
    pts = pgf.listvec3(POINTS)
    cloudGlyphs = pgf.listi32([GRECEIVER for _ in range(len(POINTS))])
    cloud = pgf.pointCloud3d(pts)
    idxPt = pgf.listItem(pts, pgv.slideri32("Index", 0, len(POINTS) - 1, 0))
    circ, center, radius = pgf.boundingCircle(cloud)

    centerList = pgf.makeList([pgf.vec3FromVec2(center)])
    centerGlyphs = pgf.listi32([GTRANSMITTER])

    pgv.glyphs("transmitters", pts, cloudGlyphs)
    pgv.glyphs("receiver", centerList, centerGlyphs)
    pgv.show("circle", circ)
    pgv.show("points", cloud)
    pgv.show("center", pgf.pointCloud3d(centerList))
    pgv.print("Point at index", idxPt)

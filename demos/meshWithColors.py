import pygalfunc as pgf
import pygalview as pgv

ptCoords = [pgv.sliderf32("ptCoord%s" % i, 0., 1., .5)[0] for i in range(3)]
normCoords = [pgv.sliderf32("normCoord%s" % i, 0., 1., .5)[0]
              for i in range(3)]


def getLambda(pts):
    pt, = pgf.vec3Var([0., 0., 0.])
    fpt, = pgf.pointCloudFarthestPt(pts, pt)
    dist, = pgf.distance(pt, fpt)
    lims, = pgf.vec2Var([0., 1.414])
    scheme, = pgf.listvec3([
        [0., 0., 1.],
        [0., 1., 0.],
        [1., 1., 0.],
        [1., 0., 0.]
    ])
    color, = pgf.mapValueToColor(dist, lims, scheme)
    return pgf.lambdaFromRegisters([pt], [color])[0]


pt, = pgf.vec3(ptCoords[0], ptCoords[1], ptCoords[2])
norm, = pgf.vec3(normCoords[0], normCoords[1], normCoords[2])
plane, = pgf.plane(pt, norm)
minCoord, = pgf.numberf32(-0.5)
maxCoord, = pgf.numberf32(0.5)
zero, = pgf.numberf32(0.)
minpt, = pgf.vec3(minCoord, minCoord, zero)
maxpt, = pgf.vec3(maxCoord, maxCoord, zero)
box2, = pgf.box2(pgf.vec2(minCoord, minCoord)[
                 0], pgf.vec2(maxCoord, maxCoord)[0])
box3, = pgf.box3(minpt, maxpt)
npts, = pgv.slideri32("Point count", 5, 50, 25)
cloud, = pgf.randomPointCloudFromBox(box3, npts)

edgeLen, = pgf.numberf32(0.1)
rect, = pgf.rectangleMesh(plane, box2, edgeLen)

colored, = pgf.meshWithVertexColorsFromLambda(rect, getLambda(cloud))

pgv.show("rectangle", rect)
pgv.show("colored", colored)

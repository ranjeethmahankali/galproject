import pygalfunc as pgf
import pygalview as pgv


def getLambda(pts):
    pt, = pgf.vec3Var([0., 0., 0.])
    fpt, = pgf.pointCloudFarthestPt(pts, pt)
    dist, = pgf.distance(pt, fpt)
    lims, = pgf.vec2Var([.5, 1.2])
    scheme, = pgf.listvec3([
        [0., 0., 1.],
        [0., 1., 0.],
        [1., 1., 0.],
        [1., 0., 0.]
    ])
    color, = pgf.mapValueToColor(dist, lims, scheme)
    return pgf.lambdaFromRegisters([pt], [color])[0]


pgv.set2dMode(True)
zero, = pgf.numberf32(0.)
one, = pgf.numberf32(1.)
pt, = pgf.vec3(zero, zero, zero)
norm, = pgf.vec3(zero, zero, one)
plane, = pgf.plane(pt, norm)
minCoord, = pgf.numberf32(-0.5)
maxCoord, = pgf.numberf32(0.5)
minpt, = pgf.vec3(minCoord, minCoord, zero)
maxpt, = pgf.vec3(maxCoord, maxCoord, zero)
box2, = pgf.box2(pgf.vec2(minCoord, minCoord)[
                 0], pgf.vec2(maxCoord, maxCoord)[0])
box3, = pgf.box3(minpt, maxpt)
npts, = pgv.slideri32("Point count", 5, 50, 25)
cloud, = pgf.randomPointCloudFromBox(box3, npts)

edgeLen, = pgf.numberf32(0.01)
rect, = pgf.rectangleMesh(plane, box2, edgeLen)

colored, = pgf.meshWithVertexColorsFromLambda(rect, getLambda(cloud))

circ, *_ = pgf.boundingCircle(cloud)

# pgv.show("rectangle", rect)
pgv.show("cloud", cloud)
pgv.show("colored", colored)
pgv.show("circle", circ)

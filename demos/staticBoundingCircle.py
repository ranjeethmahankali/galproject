import pygalfunc as pgf
import pygalview as pgv

pgv.set2dMode(True)

POINTS = [
    [0, 0, 0],
    [1, 0, 0],
    [1, 1, 0],
    [-.3, 1, 0],
    [0, -1, 0],
]

def showAllPoints():
    pts, = pgf.listvec3(POINTS)
    cloud, = pgf.pointCloud3d(pts)
    pgv.show("Points", cloud)


def showCircle(points, suffix):
    pts, = pgf.listvec3(points)
    cloud, = pgf.pointCloud3d(pts)
    circ, = pgf.boundingCircle(cloud)
    pgv.show("Circle_%s" % suffix, circ)


showAllPoints()
showCircle(POINTS[:2], "0_2")
showCircle(POINTS[:3], "0_3")
showCircle(POINTS[:4], "0_4")


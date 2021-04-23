import pygalfunc as pgf
import pygalview as pgv
import sys

pypoints = [
    [0, 0, 0],
    [1, 0, 0],
    [1, 1, 0],
    [-.3, 1, 0],
    [0, -1, 0],
]

pgv.set2dMode(True)
pts, = pgf.listvec3(pypoints)
pgv.print("My points", pts)
cloud, = pgf.pointCloud3d(pts)
circ, = pgf.boundingCircle(cloud)
# print(cloud)
pgv.show("Points", cloud)
pgv.show("Circle", circ)

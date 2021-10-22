import pygalfunc as pgf
import pygalview as pgv

minCoord = pgf.var(-1.)
maxCoord = pgf.var(1.)
minpt = pgf.vec3(minCoord, minCoord, minCoord)
maxpt = pgf.vec3(maxCoord, maxCoord, maxCoord)
box = pgf.box3(minpt, maxpt)
npts = pgv.slideri32("Point count", 10, 1000, 100)

cloud = pgf.randomPointsInBox(box, npts)
hull = pgf.convexHullFromPoints(cloud)

pgv.show("Convex Hull", hull)
pgv.show("Points", cloud)

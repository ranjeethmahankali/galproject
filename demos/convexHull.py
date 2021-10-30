import pygalfunc as pgf
import pygalview as pgv


minpt = pgf.var_vec3((-1., -1., -1.))
maxpt = pgf.var_vec3((1., 1., 1.))
box = pgf.box3(minpt, maxpt)
npts = pgv.slideri32("Point count", 10, 1000, 100)

cloud = pgf.randomPointsInBox(box, npts)
hull = pgf.convexHullFromPoints(cloud)

pgv.show("Convex Hull", hull)
pgv.show("Points", cloud)

import pygalfunc as pgf
import pygalview as pgv

minpt = pgf.var_vec3((-1., -1., 0.))
maxpt = pgf.var_vec3((1., 1., 0.))
box = pgf.box3(minpt, maxpt)

npts = pgv.slideri32("Point count", 10, 1000, 25)

cloud = pgf.randomPointsInBox(box, npts)
circ, center, radius = pgf.boundingCircle(cloud)

pgv.show("cloud", pgf.pointCloud3d(cloud))
pgv.show("circ", circ)
pgv.print("Center", center)
pgv.print("Radius", radius)

pgv.runCommands("2d")

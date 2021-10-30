import pygalfunc as pgf
import pygalview as pgv

pgv.set2dMode(True)

minpt = pgf.var_vec3((-1., -1., 0.))
maxpt = pgf.var_vec3((1., 1., 0.))
box = pgf.box3(minpt, maxpt)

npts = pgv.slideri32("Point count", 5, 50, 25)

cloud = pgf.randomPointsInBox(box, npts)
circ, *_ = pgf.boundingCircle(cloud)

pgv.show("cloud", pgf.pointCloud3d(cloud))
pgv.show("circ", circ)
pgv.print("Circle", circ)

import pygalfunc as pgf
import pygalview as pgv

pgv.set2dMode(True)

minCoord = pgf.numberf32(-1.)
maxCoord = pgf.numberf32(1.)
zero = pgf.numberf32(0.)
minpt = pgf.vec3(minCoord, minCoord, zero)
maxpt = pgf.vec3(maxCoord, maxCoord, zero)
box = pgf.box3(minpt, maxpt)

npts = pgv.slideri32("Point count", 5, 50, 25)

cloud = pgf.randomPointCloudFromBox(box, npts)
circ, *_ = pgf.boundingCircle(cloud)

pgv.show("cloud", cloud)
pgv.show("circ", circ)

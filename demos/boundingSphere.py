import pygalfunc as pgf
import pygalview as pgv


minpt = pgf.var_vec3((-1., -1., -1.))
maxpt = pgf.var_vec3((1., 1., 1.))
box = pgf.box3(minpt, maxpt)

npts = pgv.slideri32("Point count", 5, 50, 25)

cloud = pgf.randomPointsInBox(box, npts)
sphere, *_ = pgf.boundingSphere(cloud)

pgv.show("cloud", cloud)
pgv.show("sphere", sphere)

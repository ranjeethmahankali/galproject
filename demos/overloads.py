import pygalfunc as pgf
import pygalview as pgv

def sphere():
    minCoord = pgf.var(-1.)
    maxCoord = pgf.var(1.)
    minpt = pgf.vec3(minCoord, minCoord, minCoord)
    maxpt = pgf.vec3(maxCoord, maxCoord, maxCoord)
    box = pgf.box3(minpt, maxpt)
    npts = pgv.slideri32("Point count", 5, 50, 25)
    cloud = pgf.randomPointsInBox(box, npts)
    sphere, *_ = pgf.boundingSphere(cloud)
    return sphere

def mesh():
    relpath = pgf.var("../assets/bunny_large.obj")
    # relpath = pgv.textField("Relative file path");
    path = pgf.absPath(relpath)
    return pgf.loadObjFile(path)

m = mesh()
s = sphere()

# The bounds function is overloaded for both mesh and sphere.
meshbbox = pgf.bounds(m)
spbbox = pgf.bounds(s)

pgv.show("mesh", m)
pgv.show("circ", s)
pgv.show("mbbox", meshbbox)
pgv.show("cbbox", spbbox)

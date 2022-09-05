import pygalfunc as pgf
import pygalview as pgv

relpath = pgf.var_string("../../assets/bunny.obj")
path = pgf.absPath(relpath)
mesh = pgf.loadObjFile(path)
scale = pgf.var_float(10.0)
scaled = pgf.scale(mesh, scale)

center = pgv.sliderVec3("center", 0., 1., 0.)
radius = pgv.sliderf32("radius", 0., 1., .5)
sphere = pgf.sphere(center, radius)
resultMesh, resultIndices, numResults = pgf.meshSphereQuery(scaled, sphere)

pgv.print("Number of results", numResults)
pgv.show("Queried faces", resultMesh)
pgv.show("Sphere", sphere)

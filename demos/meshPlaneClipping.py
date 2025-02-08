import pygalfunc as pgf
import pygalview as pgv

# relpath = pgf.var_string("../../assets/bunny_large.obj")
relpath = pgv.textField("relpath")
path = pgf.absPath(relpath)
mesh = pgf.loadTriangleMesh(path)
scale = pgf.var_float(10.0)
scaled = pgf.scale(mesh, scale)

pt = pgv.sliderVec3("point", 0., 1., .5)
norm = pgv.sliderVec3("normal", 0., 1., .5)
plane = pgf.plane(pt, norm)
clipped = pgf.clipMesh(scaled, plane)

area = pgf.area(clipped)
centroid = pgf.centroid(clipped)

pgv.show("Plane", plane)
pgv.show("Clipped Mesh", clipped)
pgv.print("Original triangle count", pgf.numFaces(scaled))
pgv.print("Final triangle count", pgf.numFaces(clipped))
pgv.print("Mesh Area", area)
pgv.print("Mesh Centroid", centroid)

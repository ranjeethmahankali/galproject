import pygalfunc as pgf
import pygalview as pgv

# relpath = pgf.string("../assets/bunny.obj")
relpath = pgv.textField("relpath")
path = pgf.absPath(relpath)
mesh = pgf.loadObjFile(path)
scale = pgf.var(10.0)
scaled = pgf.scaleMesh(mesh, scale)

pt = pgv.sliderVec3("point", 0., 1., .5)
norm = pgv.sliderVec3("normal", 0., 1., .5)
plane = pgf.plane(pt, norm)
clipped = pgf.clipMesh(scaled, plane)

area = pgf.meshSurfaceArea(clipped)
centroid = pgf.meshCentroid(clipped)

pgv.show("Plane", plane)
pgv.show("Clipped Mesh", clipped)
pgv.print("Mesh Area", area)
pgv.print("Mesh Centroid", centroid)

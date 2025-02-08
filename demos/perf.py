"""Test the mesh plane clipping."""
import pygalfunc as pgf
import pygalview as pgv

# Load triangle mesh.
relpath = pgf.var_string("../../assets/bunny_large.obj")
# relpath = pgv.textField("relpath")
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

npts = pgv.slideri32("Point count", 10, 5000, 100)
points = pgf.randomPointsInBox(pgf.bounds(clipped), npts)
projected = pgf.closestPoints(clipped, points)

pgv.show("Plane", plane)
pgv.show("Clipped Mesh", clipped)
pgv.show("Projected points", projected)
pgv.print("Original triangle count", pgf.numFaces(scaled))
pgv.print("Final triangle count", pgf.numFaces(clipped))
pgv.print("Mesh Area", area)
pgv.print("Mesh Centroid", centroid)

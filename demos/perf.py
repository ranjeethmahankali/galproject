"""Benchmark performance of galview."""
import pygalfunc as pgf
import pygalview as pgv

# Load triangle mesh from disk and scale it by a factor of 10.
mesh = pgf.scale(
    pgf.loadTriangleMesh(
        pgf.absPath(pgf.var_string("../../assets/bunny_large.obj"))),
    pgf.var_float(10.0))
# Print statistics
pgv.show("Original mesh", mesh)
pgv.print("Triangle count", pgf.numFaces(mesh))
pgv.print("Area", pgf.area(mesh))
pgv.print("Volume", pgf.volume(mesh))

# Sample random points in the bounding box of the mesh.
npts = pgv.slideri32("Point count", 10, 10000, 100)
box = pgf.bounds(mesh)
points = pgf.randomPointsInBox(box, npts)
pgv.show("Bounding box", box)
pgv.show("Random points", points)

# Clip the mesh with a plane, project the points onto the clipped mesh,
# and print statistics.
pt = pgv.sliderVec3("point", 0., 1., .5)
norm = pgv.sliderVec3("normal", 0., 1., .5)
plane = pgf.plane(pt, norm)
clipped = pgf.clipMesh(mesh, plane)
projected = pgf.closestPoints(clipped, points)
pgv.show("Plane", plane)
pgv.show("Clipped Mesh", clipped)
pgv.show("Projected points", projected)
pgv.print("Final triangle count", pgf.numFaces(clipped))
pgv.print("Mesh Area", pgf.area(clipped))
pgv.print("Mesh Centroid", pgf.centroid(clipped))

# Translate the original mesh and filter it with a sphere.
moved = pgf.translate(clipped, pgv.sliderVec3("Translation", 0., 20., 10.))
pgv.show("Moved mesh", moved)
sphere = pgf.sphere(pgv.sliderVec3("Sphere Center", 0., 25., 15.),
                    pgv.sliderf32("Radius", 0., 10., 5.))
filtered, findices, nfaces = pgf.meshSphereQuery(moved, sphere)
pgv.show("Sphere", sphere)
pgv.show("Filtered and moved", filtered)

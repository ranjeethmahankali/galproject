import pygalfunc as pgf
import pygalview as pgv


relpath = pgf.var("../assets/bunny.obj")
path = pgf.absPath(relpath)
mesh = pgf.loadObjFile(path)
scale = pgf.var(10.0)
scaled = pgf.scaleMesh(mesh, scale)

box = pgf.meshBbox(scaled)
npts = pgv.slideri32("Point count", 100, 5000, 100)
inpts = pgf.randomPointsInBox(box, npts)

outpts = pgf.closestPointsOnMesh(scaled, inpts)

pgv.show("Box", box)
pgv.show("Mesh", scaled)
pgv.show("Query Points", inpts)
pgv.show("Closest Points", outpts)

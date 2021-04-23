import pygalfunc as pgf
import pygalview as pgv


relpath, = pgf.string("../assets/bunny.obj")
path, = pgf.absPath(relpath)
mesh, = pgf.loadObjFile(path)
scale, = pgf.numberf32(10.0)
scaled, = pgf.scaleMesh(mesh, scale)

box, = pgf.meshBbox(scaled)
npts, = pgv.slideri32("Point count", 10, 1000, 100)
cloud, = pgf.randomPointCloudFromBox(box, npts)

cloud2, = pgf.closestPointsOnMesh(scaled, cloud)

pgv.show("Box", box)
pgv.show("Mesh", scaled)
pgv.show("Query Points", cloud)
pgv.show("Closest Points", cloud2)

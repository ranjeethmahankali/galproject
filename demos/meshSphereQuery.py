import pygalfunc as pgf
import pygalview as pgv

relpath, = pgf.string("../assets/bunny.obj")
path, = pgf.absPath(relpath)
mesh, = pgf.loadObjFile(path)
scale, = pgf.numberf32(10.0)
scaled, = pgf.scaleMesh(mesh, scale)

centerCoords = [pgv.sliderf32("center-coord%s" % i, 0., 1., 0.)[0] for i in range(3)]
center, = pgf.vec3(centerCoords[0], centerCoords[1], centerCoords[2])
radius, = pgv.sliderf32("radius", 0., 1., .5)
sphere, = pgf.sphere(center, radius)
resultMesh, resultIndices, numResults = pgf.meshSphereQuery(scaled, sphere)

pgv.print("Number of results", numResults)
pgv.show("Queried faces", resultMesh)
pgv.show("Sphere", sphere)

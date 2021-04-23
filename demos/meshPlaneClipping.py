import pygalfunc as pgf
import pygalview as pgv

ptCoords = [pgv.sliderf32("ptCoord%s" % i, 0., 1., .5)[0] for i in range(3)]
normCoords = [pgv.sliderf32("normCoord%s" % i, 0., 1., .5)[0]
              for i in range(3)]

relpath, = pgf.string("../assets/bunny.obj")
path, = pgf.absPath(relpath)
mesh, = pgf.loadObjFile(path)
scale, = pgf.numberf32(10.0)
scaled, = pgf.scaleMesh(mesh, scale)

pt, = pgf.vec3(ptCoords[0], ptCoords[1], ptCoords[2])
norm, = pgf.vec3(normCoords[0], normCoords[1], normCoords[2])
plane, = pgf.plane(pt, norm)
clipped,  = pgf.clipMesh(scaled, plane)

area, = pgf.meshSurfaceArea(clipped)
centroid,  = pgf.meshCentroid(clipped)

pgv.show("Plane", plane)
pgv.show("Clipped Mesh", clipped)
pgv.print("Mesh Area", area)
pgv.print("Mesh Centroid", centroid)

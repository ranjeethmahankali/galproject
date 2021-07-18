import pygalfunc as pgf
import pygalview as pgv

ptCoords = [pgv.sliderf32("ptCoord%s" % i, 0., 1., .5)[0] for i in range(3)]
normCoords = [pgv.sliderf32("normCoord%s" % i, 0., 1., .5)[0]
              for i in range(3)]

pt, = pgf.vec3(ptCoords[0], ptCoords[1], ptCoords[2])
norm, = pgf.vec3(normCoords[0], normCoords[1], normCoords[2])
plane, = pgf.plane(pt, norm)
bounds, = pgf.box2(pgf.vec2(pgf.numberf32(-0.5)[0], pgf.numberf32(-0.5)[0])[
                   0], pgf.vec2(pgf.numberf32(0.5)[0], pgf.numberf32(0.5)[0])[0])
edgeLen, = pgf.numberf32(0.1)

rect, = pgf.rectangleMesh(plane, bounds, edgeLen)

pgv.show("rectangle", rect)

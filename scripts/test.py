import sys
import pygalfunc as pgf
import pygalview as pgv

sliders = [pgv.sliderf32("Slider %s" % i, 0.0, 1.0, 0.5)[0] for i in range(4)]

center, = pgf.vec3(sliders[0], sliders[1], sliders[2])
sphere, = pgf.sphere(center, sliders[3])

pgv.show(sphere)

# path, = pgf.string("/home/rnjth94/dev/GeomAlgoLib/assets/bunny.obj")
# mesh, = pgf.loadObjFile(path)
# x, y, z = pgf.meshCentroid(mesh)

# print(x, y, z)

# print("(%.3f, %.3f, %.3f)" %
#       (pgf.readFloat(x), pgf.readFloat(y), pgf.readFloat(z)))

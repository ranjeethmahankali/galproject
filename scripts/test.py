import sys
# sys.path.append("/home/rnjth94/dev/GeomAlgoLib/build/")
import pygalfunc as pg

path, = pg.string("/home/rnjth94/dev/GeomAlgoLib/assets/bunny.obj")
mesh, = pg.loadObjFile(path)
x, y, z = pg.meshCentroid(mesh)

print(x, y, z)

print("(%.3f, %.3f, %.3f)" %
      (pg.readFloat(x), pg.readFloat(y), pg.readFloat(z)))

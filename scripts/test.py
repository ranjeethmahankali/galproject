import sys
sys.path.append("/home/rnjth94/dev/GeomAlgoLib/build/")

import pygalfunc as pg

mesh = pg.loadObjFile("/home/rnjth94/dev/GeomAlgoLib/assets/bunny.obj")

x, y, z = pg.meshCentroid(mesh)

print("(%.3f, %.3f, %.3f)" % (x, y, z))
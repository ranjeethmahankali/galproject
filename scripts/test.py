import sys
sys.path.append("/home/rnjth94/dev/GeomAlgoLib/build/")

import pygalfunc as pg

mesh = pg.loadObjFile("/home/rnjth94/dev/GeomAlgoLib/assets/bunny.obj")

center = pg.meshCentroid(mesh)

print("(%.3f, %.3f, %.3f)" % (center.x, center.y, center.z))
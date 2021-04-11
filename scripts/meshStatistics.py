from galutils import *

AppendBinPath()

import pygalfunc as pgf

path, = pgf.string(GetRelativePath("assets/bunny_large.obj"))
mesh, = pgf.loadObjFile(path)

area, = pgf.meshSurfaceArea(mesh)
volume, = pgf.meshVolume(mesh)

print("Loading mesh from %s" % pgf.readstring(path))
print("Mesh Area: %.3f" % pgf.readf32(area))
print("Mesh Volume: %.3f" % pgf.readf32(volume))
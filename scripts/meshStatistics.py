from galutils import *

# fmt: off
AppendBinPath()
import pygalfunc as pgf
# fmt: on

path = pgf.string(GetRelativePath("assets/bunny_large.obj"))
mesh = pgf.loadObjFile(path)

area = pgf.meshSurfaceArea(mesh)
volume = pgf.meshVolume(mesh)

print("Loading mesh from %s" % pgf.read(path))
print("Mesh Area: %.3f" % pgf.read(area))
print("Mesh Volume: %.3f" % pgf.read(volume))

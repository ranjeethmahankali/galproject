from galutils import *

# fmt: off
# AppendBinPath("Debug")
AppendBinPath()
import pygalfunc as pgf
# fmt: on

def getMeshStatistics(mesh):
    area = pgf.meshSurfaceArea(mesh)
    volume = pgf.meshVolume(mesh)
    return area, volume


def printStatistics(mesh):
    area, volume = getMeshStatistics(mesh)
    print("Mesh Area: %.3f" % pgf.read(area))
    print("Mesh Volume: %.3f" % pgf.read(volume))


def processMesh(path):
    print("Loading mesh from %s" % pgf.read(path))
    printStatistics(pgf.loadObjFile(path))


processMesh(pgf.var(GetRelativePath("assets/bunny_large.obj")))


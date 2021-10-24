import pygalfunc as pgf
from os import path


def assetPath(filename):
    return path.join(path.dirname(path.dirname(path.realpath(__file__))),
                     "assets", filename)


def getMeshStatistics(mesh):
    area = pgf.area(mesh)
    volume = pgf.volume(mesh)
    return area, volume


def printStatistics(mesh):
    area, volume = getMeshStatistics(mesh)
    print("Mesh Area: %.3f" % pgf.read(area))
    print("Mesh Volume: %.3f" % pgf.read(volume))


def processMesh(path):
    print("Loading mesh from %s" % pgf.read(path))
    printStatistics(pgf.loadObjFile(path))


processMesh(pgf.var(assetPath("bunny_large.obj")))

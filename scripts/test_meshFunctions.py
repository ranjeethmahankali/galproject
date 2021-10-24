import pygalfunc as pgf
import testUtil as tu


def meshStatistics(assetFileName):
    fpath = pgf.var(tu.assetPath(assetFileName))
    mesh = pgf.loadObjFile(fpath)
    area = pgf.area(mesh)
    volume = pgf.volume(mesh)
    return pgf.read(area), pgf.read(volume)


def test_meshStatistics():
    area, volume = meshStatistics("bunny_large.obj")
    assert abs(area - 23.27821922302246) < 1e-5
    assert abs(volume - 6.039211750030518) < 1e-5


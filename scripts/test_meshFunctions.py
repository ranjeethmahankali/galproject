import pygalfunc as pgf
import testUtil as tu


def loadMesh(assetFileName):
    fpath = pgf.var(tu.assetPath(assetFileName))
    return pgf.loadObjFile(fpath)


def loadLargeBunny():
    return loadMesh("bunny_large.obj")


def meshStatistics(mesh):
    area = pgf.area(mesh)
    volume = pgf.volume(mesh)
    nfaces = pgf.numFaces(mesh)
    nverts = pgf.numVertices(mesh)
    return pgf.read(area), pgf.read(volume), pgf.read(nfaces), pgf.read(nverts)


def test_meshStatistics():
    area, volume, nfaces, nverts = meshStatistics(loadLargeBunny())
    assert abs(area - 23.27821922302246) < 1e-5
    assert abs(volume - 6.039211750030518) < 1e-5
    assert nfaces == 69630
    assert nverts == 34817


def test_meshPlaneClipping():
    mesh = loadLargeBunny()
    half = pgf.var(0.5)
    plane = pgf.plane(pgf.vec3(half, half, half), pgf.vec3(half, half, half))
    clipped = pgf.clipMesh(mesh, plane)
    
    area = pgf.read(pgf.area(clipped))
    nfaces = pgf.read(pgf.numFaces(clipped))
    nverts = pgf.read(pgf.numVertices(clipped))

    assert abs(area - 19.79486274) < 1e-5
    assert nfaces == 59387
    assert nverts == 30013


def test_meshCentroid():
    mesh = loadLargeBunny()
    centroid = pgf.read(pgf.centroid(mesh))
    assert centroid == [-0.4187779128551483, -0.21727733314037323, 0.7404823899269104]


def test_scaleMesh():
    mesh = loadLargeBunny()
    factor = 10.0
    scaled = pgf.scale(mesh, pgf.var(factor))
    meshes = [mesh, scaled]

    areas = [pgf.read(pgf.area(m)) for m in meshes]
    volumes = [pgf.read(pgf.volume(m)) for m in meshes]
    vcounts = [pgf.read(pgf.numVertices(m)) for m in meshes]
    fcounts = [pgf.read(pgf.numFaces(m)) for m in meshes]

    assert (abs(areas[1] - pow(factor, 2) * areas[0]) / areas[1]) < 1e-5
    assert (abs(volumes[1] - pow(factor, 3) * volumes[0]) / volumes[1]) < 1e-5
    assert vcounts[0] == vcounts[1]
    assert fcounts[0] == fcounts[1]

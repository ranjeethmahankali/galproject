import pygalfunc as pgf
import testUtil as tu


def loadMesh(assetFileName):
    fpath = pgf.var(tu.assetPath(assetFileName))
    return pgf.loadObjFile(fpath)


def loadLargeBunny():
    return loadMesh("bunny_large.obj")


def loadSmallBunny():
    return loadMesh("bunny.obj")


def test_meshCentroid():
    mesh = loadLargeBunny()
    centroid = pgf.read(pgf.centroid(mesh))
    assert tu.equalf(
        centroid, [-0.4187779128551483, -0.21727733314037323, 0.7404823899269104])


def test_meshStatistics():
    mesh = loadLargeBunny()
    area = pgf.read(pgf.area(mesh))
    volume = pgf.read(pgf.volume(mesh))
    nfaces = pgf.read(pgf.numFaces(mesh))
    nverts = pgf.read(pgf.numVertices(mesh))
    assert tu.equalf(area, 23.27821922302246)
    assert tu.equalf(volume, 6.039211750030518)
    assert nfaces == 69630
    assert nverts == 34817


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


def test_meshBounds():
    assetFiles = ["bunny_large.obj", "bunny.obj"]
    fpaths = pgf.var("")
    pgf.assign(fpaths, [tu.assetPath(a) for a in assetFiles])
    meshes = pgf.loadObjFile(fpaths)
    bounds = pgf.bounds(meshes)
    pmins, pmaxs = pgf.boxPoints(bounds)

    vpmins = pgf.read(pmins)
    vpmaxs = pgf.read(pmaxs)
    assert tu.equalf(vpmins,
                     [[-1.8937978744506836, -1.1759940385818481, -0.3402520418167114],
                      [-0.09438041597604752, -0.058714643120765686, 0.033309899270534515]])
    assert tu.equalf(vpmaxs,
                     [[1.2201818227767944, 1.2374719381332397, 2.746419906616211],
                      [0.060778796672821045, 0.061679162085056305, 0.18699601292610168]])


def test_meshPlaneClipping():
    mesh = loadLargeBunny()
    half = pgf.var(0.5)
    plane = pgf.plane(pgf.vec3(half, half, half), pgf.vec3(half, half, half))
    clipped = pgf.clipMesh(mesh, plane)

    area = pgf.read(pgf.area(clipped))
    nfaces = pgf.read(pgf.numFaces(clipped))
    nverts = pgf.read(pgf.numVertices(clipped))

    assert tu.equalf(area, 19.79486274)
    assert nfaces == 59387
    assert nverts == 30013


def test_meshSphereQuery():
    mesh = pgf.scale(loadSmallBunny(), pgf.var(10.0))
    zero = pgf.var(0.)
    center = pgf.vec3(zero, zero, zero)
    radius = pgf.var(.5)
    sphere = pgf.sphere(center, radius)

    rmesh, rindices, rcount = pgf.meshSphereQuery(mesh, sphere)

    nfaces = pgf.read(rcount)
    area = pgf.read(pgf.area(rmesh))
    assert tu.equalf(area, 0.4436888098716736)
    assert nfaces == 292


if __name__ == "__main__":
    test_closestPoints()

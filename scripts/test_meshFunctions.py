import pygalfunc as pgf
import testUtil as tu


def test_meshCentroid():
    mesh = tu.loadLargeBunny()
    centroid = pgf.read(pgf.centroid(mesh))
    assert tu.equalf(
        centroid,
        [-0.46018099784851074, -0.17610645294189453, 0.8226389288902283])


def test_meshStatistics():
    mesh = tu.loadLargeBunny()
    area = pgf.read(pgf.area(mesh))
    volume = pgf.read(pgf.volume(mesh))
    nfaces = pgf.read(pgf.numFaces(mesh))
    nverts = pgf.read(pgf.numVertices(mesh))
    assert tu.equalf(area, 23.27821922302246)
    assert tu.equalf(volume, 6.039211750030518)
    assert nfaces == 69630
    assert nverts == 34817


def test_scaleMesh():
    mesh = tu.loadLargeBunny()
    factor = 10.0
    scaled = pgf.scale(mesh, pgf.var_float(factor))
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
    fpaths = pgf.var_string("")
    pgf.assign(fpaths, [tu.assetPath(a) for a in assetFiles])
    meshes = pgf.loadObjFile(fpaths)
    bounds = pgf.bounds(meshes)
    pmins, pmaxs = pgf.boxPoints(bounds)
    vpmins = pgf.read(pmins)
    vpmaxs = pgf.read(pmaxs)
    assert tu.equalf(
        vpmins,
        [[-1.8937978744506836, -1.1759940385818481, -0.3402520418167114],
         [-0.09438041597604752, -0.058714643120765686, 0.033309899270534515]])
    assert tu.equalf(
        vpmaxs,
        [[1.2201818227767944, 1.2374719381332397, 2.746419906616211],
         [0.060778796672821045, 0.061679162085056305, 0.18699601292610168]])


def test_meshPlaneClipping():
    mesh = tu.loadLargeBunny()
    half = pgf.var_float(0.5)
    plane = pgf.plane(pgf.vec3(half, half, half), pgf.vec3(half, half, half))
    clipped = pgf.clipMesh(mesh, plane)
    area = pgf.read(pgf.area(clipped))
    nfaces = pgf.read(pgf.numFaces(clipped))
    nverts = pgf.read(pgf.numVertices(clipped))
    assert tu.equalf(area, 19.794883728027344)
    assert nfaces == 59387
    assert nverts == 30013


def test_meshSphereQuery():
    mesh = pgf.scale(tu.loadSmallBunny(), pgf.var_float(10.0))
    zero = pgf.var_float(0.)
    center = pgf.vec3(zero, zero, zero)
    radius = pgf.var_float(.5)
    sphere = pgf.sphere(center, radius)
    rmesh, rindices, rcount = pgf.meshSphereQuery(mesh, sphere)
    nfaces = pgf.read(rcount)
    area = pgf.read(pgf.area(rmesh))
    assert tu.equalf(area, 0.4436888098716736)
    assert nfaces == 292


def test_closestPointsOnMesh():
    mesh = pgf.scale(tu.loadSmallBunny(), pgf.var_float(10.))
    inpts = pgf.var_vec3([
        (0.359824538230896, -0.012389957904815674, 0.3581507205963135),
        (-0.3318827152252197, 0.1699751615524292, 0.7063822150230408),
        (0.27124643325805664, -0.14796850085258484, 0.5440048575401306),
        (0.2950490713119507, 0.0309564471244812, 1.5690069198608398),
        (0.470700740814209, 0.559279203414917, 0.5738930106163025),
        (-0.6372849941253662, 0.5158957242965698, 0.9492948055267334),
        (-0.42367517948150635, 0.17821109294891357, 0.5325688719749451),
        (0.24817490577697754, 0.27643465995788574, 0.5003229975700378),
        (-0.5128110647201538, -0.4166657030582428, 1.868307113647461),
        (-0.08426868915557861, 0.14360648393630981, 0.6685295104980469)
    ])
    outpts = pgf.closestPoints(mesh, inpts)
    expected = [
        (0.35241958498954773, -0.022721359506249428, 0.3823484480381012),
        (-0.3473210334777832, 0.18098655343055725, 0.6997533440589905),
        (0.22252899408340454, -0.13979701697826385, 0.36583301424980164),
        (0.1671879142522812, -0.029879290610551834, 1.274683952331543),
        (0.22475071251392365, 0.2590671181678772, 0.5857457518577576),
        (-0.5479510426521301, 0.21973726153373718, 0.9220386743545532),
        (-0.423895001411438, 0.11360426992177963, 0.5336558818817139),
        (0.21963083744049072, 0.23689928650856018, 0.5136839151382446),
        (-0.6122115254402161, -0.3117043375968933, 1.5369797945022583),
        (-0.03959565982222557, 0.34772586822509766, 0.666935384273529)
    ]
    assert tu.equalf(expected, pgf.read(outpts))


def test_rectangleMesh():
    plane = pgf.plane(pgf.var_vec3((0., 0., 0.)), pgf.var_vec3((1., 1., 0.)))
    box = pgf.box2(pgf.var_vec2((0., 0.)), pgf.var_vec2((15., 12.)))
    edgeLn = pgf.var_float(1.)
    rect = pgf.rectangleMesh(plane, box, edgeLn)
    area = pgf.area(rect)
    nfaces = pgf.numFaces(rect)
    nverts = pgf.numVertices(rect)
    assert 360 == pgf.read(nfaces)
    assert 208 == pgf.read(nverts)
    assert tu.equalf(180., pgf.read(area))


if __name__ == "__main__":
    test_rectangleMesh()

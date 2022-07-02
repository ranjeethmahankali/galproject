import pygalfunc as pgf
import testUtil as tu
import random


def test_graft():
    random.seed(42)
    intree = pgf.var_int()
    out = pgf.graft(intree)
    for _ in range(5):
        vals = [random.randint(23, 345) for __ in range(10)]
        expected = [[v] for v in vals]
        pgf.assign(intree, vals)
        assert tu.equal(expected, pgf.read(out))
    for _ in range(5):
        vals = [[random.randint(23, 345) for __ in range(5)]
                for ___ in range(5)]
        expected = [[[v] for v in vl] for vl in vals]
        pgf.assign(intree, vals)
        assert tu.equal(expected, pgf.read(out))


def test_flatten():
    random.seed(42)
    intree = pgf.var_int()
    outtree = pgf.flatten(intree)
    for _ in range(5):
        vals = [[random.randint(23, 234)] for __ in range(10)]
        pgf.assign(intree, vals)
        expected = [item for subl in vals for item in subl]
        assert tu.equal(expected, pgf.read(outtree))
        vals = [[[random.randint(23, 234) for __ in range(5)]
                 for ___ in range(5)] for _____ in range(5)]
        pgf.assign(intree, vals)
        expected = [item for l0 in vals for l1 in l0 for item in l1]
        assert tu.equal(expected, pgf.read(outtree))


def test_cachedOffsetDataRegression():
    # This tests for a regression (lack of) happened while working on PR # 53.
    pt = pgf.var_vec3((0., 0., 0.))
    norm = pgf.var_vec3((0., 0., 1.))
    plane = pgf.plane(pt, norm)
    minpt = pgf.var_vec3((-.5, -.5, 0.))
    maxpt = pgf.var_vec3((.5, .5, 0.))
    box2 = pgf.box2(pgf.var_vec2((-.5, -.5)), pgf.var_vec2((.5, .5)))
    box3 = pgf.box3(minpt, maxpt)
    npts = pgf.var_int(4)
    cloud = pgf.randomPointsInBox(box3, npts)
    edgeLen = pgf.var_float(1)
    rect = pgf.rectangleMesh(plane, box2, edgeLen)
    distances = pgf.distance(pgf.graft(pgf.vertices(rect)), cloud)
    pdists = pgf.read(pgf.flatten(distances))
    assert len(pdists) == 16


if __name__ == "__main__":
    test_flatten()

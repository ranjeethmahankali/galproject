import pygalfunc as pgf
import testUtil as tu
import math
import random


def unaryFloatOpTest(expFn, pgfn, minval=0., maxval=9.):
    random.seed(42)
    x = pgf.var_float()
    s = pgfn(x)

    for _ in range(20):
        vx = random.uniform(minval, maxval)
        expected = expFn(vx)
        pgf.assign(x, vx)
        tu.assertEqualf(expected, pgf.read(s), 1e-3)

    vxs = [random.uniform(minval, maxval) for _ in range(20)]
    expectedvals = [expFn(v) for v in vxs]
    pgf.assign(x, vxs)
    tu.assertEqualf(expectedvals, pgf.read(s), 1e-3)


def binaryIntOpTest(expFn, pgfn, minval=2, maxval=124):
    random.seed(42)
    a = pgf.var_int()
    b = pgf.var_int()
    r = pgfn(a, b)

    for _ in range(20):
        va = random.randint(minval, maxval)
        vb = random.randint(minval, maxval)
        expected = expFn(va, vb)
        pgf.assign(a, va)
        pgf.assign(b, vb)
        assert tu.equal(expected, pgf.read(r))

    vas = [random.randint(minval, maxval) for _ in range(20)]
    vbs = [random.randint(minval, maxval) for _ in range(20)]
    expectedVals = [expFn(va, vb) for va, vb in zip(vas, vbs)]
    pgf.assign(a, vas)
    pgf.assign(b, vbs)
    assert tu.equal(expectedVals, pgf.read(r))


def binaryFloatOpTest(expFn, pgfn, minval=2., maxval=124.5):
    random.seed(42)
    a = pgf.var_float()
    b = pgf.var_float()
    r = pgfn(a, b)

    for _ in range(20):
        va = random.uniform(minval, maxval)
        vb = random.uniform(minval, maxval)
        expected = expFn(va, vb)
        pgf.assign(a, va)
        pgf.assign(b, vb)
        assert tu.equalf(expected, pgf.read(r), 1e-3)

    vas = [random.uniform(minval, maxval) for _ in range(20)]
    vbs = [random.uniform(minval, maxval) for _ in range(20)]
    expectedVals = [expFn(va, vb) for va, vb in zip(vas, vbs)]
    pgf.assign(a, vas)
    pgf.assign(b, vbs)
    assert tu.equalf(expectedVals, pgf.read(r), 1e-3)


def test_sin():
    unaryFloatOpTest(math.sin, pgf.sin)


def test_cos():
    unaryFloatOpTest(math.cos, pgf.cos)


def test_tan():
    unaryFloatOpTest(math.tan, pgf.tan, -1.5, 1.5)


def test_arcsin():
    unaryFloatOpTest(math.asin, pgf.arcsin, -1., 1.)


def test_arccos():
    unaryFloatOpTest(math.acos, pgf.arccos, -1., 1.)


def test_arctan():
    unaryFloatOpTest(math.atan, pgf.arctan, -9., 9.)


def test_sqrt():
    unaryFloatOpTest(math.sqrt, pgf.sqrtf32)


def test_add():
    def expFn(a, b): return a + b
    binaryIntOpTest(expFn, pgf.add)
    binaryFloatOpTest(expFn, pgf.add)


def test_sub():
    def expfn(a, b): return a - b
    binaryIntOpTest(expfn, pgf.sub)
    binaryFloatOpTest(expfn, pgf.sub)


def test_mul():
    def expfn(a, b): return a * b
    binaryIntOpTest(expfn, pgf.mul)
    binaryFloatOpTest(expfn, pgf.mul)


def test_div():
    binaryIntOpTest(lambda a, b: a//b, pgf.div)
    binaryFloatOpTest(lambda a, b: a / b, pgf.div)


def test_seriesInt():
    random.seed(42)
    valrange = (23, 256)
    steprange = (2, 9)
    valstart = random.randint(valrange[0], valrange[1])
    valstep = random.randint(steprange[0], steprange[1])
    valcount = random.randint(valrange[0], valrange[1])
    valstop = valstart + valstep * valcount

    start = pgf.var_int(valstart)
    step = pgf.var_int(valstep)
    count = pgf.var_int(valcount)
    series = pgf.series(start, step, count)

    assert tu.equal(range(valstart, valstop, valstep), pgf.read(series))


def test_seriesFloat():
    random.seed(42)
    valrange = (22.345, 223.66)
    steprange = (1.778, 9.56)
    valstart = random.uniform(valrange[0], valrange[1])
    valstep = random.uniform(steprange[0], steprange[1])
    valcount = random.randint(25, 98)
    valstop = valstart + valstep * valcount

    start = pgf.var_float(valstart)
    step = pgf.var_float(valstep)
    count = pgf.var_int(valcount)
    series = pgf.series(start, step, count)

    expected = [(valstart + i * valstep) for i in range(valcount)]
    tu.assertEqualf(expected, pgf.read(series), 1e-3)


if __name__ == "__main__":
    test_seriesFloat()

import pygalfunc as pgf
import testUtil as tu
import math
import random
import itertools


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


def test_listSumInt():
    random.seed(42)
    valrange = (23, 345)
    vals = [random.randint(valrange[0], valrange[1]) for _ in range(25)]

    lst = pgf.var_int(vals)
    result = pgf.listSum(lst)

    assert tu.equal(sum(vals), pgf.read(result))


def test_listSumFloat():
    random.seed(42)
    valrange = (22.345, 223.66)
    vals = [random.uniform(valrange[0], valrange[1]) for _ in range(25)]

    lst = pgf.var_float(vals)
    result = pgf.listSum(lst)

    assert tu.equalf(sum(vals), pgf.read(result), 1e-3)


def repeatGenericTest(val, pgvarfn, comparefn):
    random.seed(42)
    count = random.randint(23, 345)
    vals = [val for _ in range(count)]

    rval = pgvarfn(val)
    result = pgf.repeat(rval, pgf.var_int(count))

    assert comparefn(vals, pgf.read(result))


def test_repeat():
    def compareInt(a, b): return tu.equal(a, b)
    repeatGenericTest(int(7), pgf.var_int, compareInt)

    def compareFloat(a, b): return tu.equalf(a, b)
    repeatGenericTest(7.34, pgf.var_float, compareFloat)

    repeatGenericTest((2.45, 3.67), pgf.var_vec2, compareFloat)
    repeatGenericTest((2.45, 3.67, 45.4), pgf.var_vec3, compareFloat)


def listItemGenericTest(vals, pgvarfn, comparefn):
    rvals = pgvarfn(vals)
    rindex = pgf.var_int()
    result = pgf.listItem(rvals, rindex)

    random.seed(42)
    for _ in range(25):
        index = random.randint(0, len(vals) - 1)
        pgf.assign(rindex, index)
        assert comparefn(vals[index], pgf.read(result))


def test_listItem():
    def compareInt(a, b): return tu.equal(a, b)
    listItemGenericTest(list(range(2, 23)), pgf.var_int, compareInt)

    random.seed(42)
    valrange = (22.345, 223.66)
    vals = [random.uniform(valrange[0], valrange[1]) for _ in range(25)]
    def compareFloat(a, b): return tu.equalf(a, b)
    listItemGenericTest(vals, pgf.var_float, compareFloat)


def test_listLength():
    random.seed(42)
    valrange = (23, 345)
    rvals = pgf.var_int()
    rstart = pgf.var_int()
    rstop = pgf.var_int()
    slist = pgf.subList(rvals, rstart, rstop)
    for _ in range(20):
        vals = [random.randint(valrange[0], valrange[1]) for _ in range(25)]
        irange = (random.randint(0, len(vals) - 1),
                  random.randint(0, len(vals) - 1))
        irange = sorted(irange)
        expected = vals[irange[0]: irange[1]]
        # print(len(vals), irange[0], irange[1])
        pgf.assign(rvals, vals)
        pgf.assign(rstart, irange[0])
        pgf.assign(rstop, irange[1])
        tu.assertEqual(expected, pgf.read(slist))


def test_dispatch():
    random.seed(42)
    valrange = (23, 345)
    rvals = pgf.var_int()
    rpattern = pgf.var_bool()
    rtvals, rfvals = pgf.dispatch(rvals, rpattern)
    for _ in range(20):
        nItems = 25
        vals = [random.randint(valrange[0], valrange[1])
                for _ in range(nItems)]
        pattern = [random.randint(valrange[0], valrange[1]) %
                   2 == 0 for _ in range(nItems)]
        tvals = [v for v, p in zip(vals, pattern) if p]
        fvals = [v for v, p in zip(vals, pattern) if not p]

        pgf.assign(rvals, vals)
        pgf.assign(rpattern, pattern)
        assert tu.equal(tvals, pgf.read(rtvals))
        assert tu.equal(fvals, pgf.read(rfvals))


def test_combinations():
    random.seed(42)
    valrange = (23, 345)
    rvals = pgf.var_int()
    rnumc = pgf.var_int()
    rcombs = pgf.combinations(rvals, rnumc)
    for _ in range(5):
        nItems = random.randint(23, 29)
        numc = random.randint(3, 5)
        vals = [random.randint(valrange[0], valrange[1]) for _ in range(nItems)]
        combs = list(itertools.combinations(vals, numc))

        pgf.assign(rvals, vals)
        pgf.assign(rnumc, numc)
        assert tu.equal(combs, pgf.read(rcombs))


if __name__ == "__main__":
    test_combinations()

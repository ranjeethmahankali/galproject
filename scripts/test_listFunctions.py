import pygalfunc as pgf
import testUtil as tu
import random
import math
import itertools

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


def test_sort():
    random.seed(42)
    rvals = pgf.var_int()
    rkeys = pgf.var_int()
    rsorted = pgf.sort(rvals, rkeys)
    lvals = []
    lkeys = []
    for _ in range(10):
        nItems = random.randint(75, 155)
        vals = list(range(nItems))
        keys = list(range(nItems))
        random.shuffle(vals)
        random.shuffle(keys)
        lvals.append(vals)
        lkeys.append(keys)
        expected = [x for x, _ in sorted(zip(vals, keys), key=lambda pair: pair[1])]
        
        pgf.assign(rvals, vals)
        pgf.assign(rkeys, keys)
        assert tu.equal(expected, pgf.read(rsorted))

    # Test the same with tree combinatorics.
    expected = [[x for x, _ in sorted(zip(v, k), key=lambda pair: pair[1])]
                for v, k in zip(lvals, lkeys)]
    pgf.assign(rvals, lvals)
    pgf.assign(rkeys, lkeys)
    assert tu.equal(expected, pgf.read(rsorted))


if __name__=="__main__":
    test_sort()

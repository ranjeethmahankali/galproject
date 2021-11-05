import pygalfunc as pgf
import testUtil as tu
import random


def test_itemListMapping():
    random.seed(42)
    singleval = 42
    single = pgf.var_int(singleval)
    lst = pgf.var_int()
    out = pgf.add(single, lst)
    for _ in range(10):
        vals = list(range(12))
        random.shuffle(vals)
        expected = [singleval + v for v in vals]

        pgf.assign(lst, vals)
        assert tu.equal(expected, pgf.read(out))


def test_treeInputs():
    random.seed(42)
    vals = [[random.randint(23, 345) for _ in range(10)] for _ in range(10)]
    rvals = pgf.var_int()
    rsum = pgf.treeSum(rvals)
    expected = sum([sum(v) for v in vals])

    pgf.assign(rvals, vals)
    assert tu.equal(expected, pgf.read(rsum))


def test_heterogenousTrees():
    random.seed(42)
    vals1 = [[2], [3]]
    vals2 = [4, 5]
    expected = [[8, 10], [12, 15]]

    rvals1 = pgf.var_int(vals1)
    rvals2 = pgf.var_int(vals2)
    prod = pgf.mul(rvals1, rvals2)
    assert tu.equal(expected, pgf.read(prod))


if __name__=="__main__":
    test_heterogenousTrees()

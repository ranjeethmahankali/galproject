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
        vals = [[random.randint(23, 345) for __ in range(5)] for ___ in range(5)]
        expected = [[[v] for v in vl] for vl in vals]
        pgf.assign(intree, vals)
        assert tu.equal(expected, pgf.read(out))


if __name__=="__main__":
    test_graft()

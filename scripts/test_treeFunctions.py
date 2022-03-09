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


if __name__ == "__main__":
    test_flatten()

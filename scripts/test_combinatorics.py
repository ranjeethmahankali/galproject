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


if __name__=="__main__":
    test_itemListMapping()

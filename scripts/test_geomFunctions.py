import pygalfunc as pgf
import testUtil as tu
import random
import math


def test_distance3():
    p1 = pgf.var_vec3()
    p2 = pgf.var_vec3()
    dist = pgf.distance(p1, p2)
    for _ in range(20):
        val1 = (random.uniform(1.2, 12.5), random.uniform(2.5, 15.6),
                random.uniform(22.3, 55.6))
        val2 = (random.uniform(1.26, 22.5), random.uniform(1.5, 13.9),
                random.uniform(23.5, 63.7))
        dval = math.sqrt(
            math.pow(val1[0] - val2[0], 2.) + math.pow(val1[1] - val2[1], 2.) +
            math.pow(val1[2] - val2[2], 2.))
        pgf.assign(p1, val1)
        pgf.assign(p2, val2)

        assert tu.equalf(pgf.read(dist), dval)


def test_distance2():
    p1 = pgf.var_vec2()
    p2 = pgf.var_vec2()
    dist = pgf.distance(p1, p2)
    for _ in range(20):
        val1 = (random.uniform(1.2, 12.5), random.uniform(2.5, 15.6))
        val2 = (random.uniform(1.26, 22.5), random.uniform(1.5, 13.9))
        dval = math.sqrt(
            math.pow(val1[0] - val2[0], 2.) + math.pow(val1[1] - val2[1], 2.))
        pgf.assign(p1, val1)
        pgf.assign(p2, val2)

        assert tu.equalf(pgf.read(dist), dval)

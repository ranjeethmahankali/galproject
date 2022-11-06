from os import path
import pygalfunc as pgf
from collections.abc import Iterable


def assetPath(filename):
    return path.join(path.dirname(path.dirname(path.realpath(__file__))),
                     "assets", filename)


def loadMesh(assetFileName):
    fpath = pgf.var_string(assetPath(assetFileName))
    return pgf.loadObjFile(fpath)


def loadLargeBunny():
    return loadMesh("bunny_large.obj")


def loadSmallBunny():
    return loadMesh("bunny.obj")


def emptyOrNone(a):
    return (isinstance(a, Iterable) and len(a) == 0) or a is None


def equalf(a, b, eps=1e-5):
    if emptyOrNone(a) and emptyOrNone(b):
        return True
    if isinstance(a, Iterable) and isinstance(b, Iterable):
        return all([equalf(u, v, eps) for u, v in zip(a, b)])
    elif not isinstance(a, Iterable) and not isinstance(b, Iterable):
        return abs(float(a) - float(b)) < eps
    else:
        return False


def equal(a, b):
    if emptyOrNone(a) and emptyOrNone(b):
        return True
    if isinstance(a, Iterable) and isinstance(b, Iterable):
        return all([equal(u, v) for u, v in zip(a, b)])
    elif not isinstance(a, Iterable) and not isinstance(b, Iterable):
        return a == b
    else:
        return False


def assertEqualf(a, b, eps=1e-5):
    if emptyOrNone(a) and emptyOrNone(b):
        return
    if isinstance(a, Iterable) and isinstance(b, Iterable):
        for u, v in zip(a, b):
            assertEqualf(u, v, eps)
    elif not isinstance(a, Iterable) and not isinstance(b, Iterable):
        assert abs(float(a) - float(b)) < eps
    else:
        assert False


def assertEqual(a, b):
    if emptyOrNone(a) and emptyOrNone(b):
        return
    if isinstance(a, Iterable) and isinstance(b, Iterable):
        for u, v in zip(a, b):
            assertEqual(u, v)
    elif not isinstance(a, Iterable) and not isinstance(b, Iterable):
        assert a == b
    else:
        assert False

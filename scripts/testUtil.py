from os import path
from collections.abc import Iterable


def assetPath(filename):
    return path.join(path.dirname(path.dirname(path.realpath(__file__))),
                     "assets", filename)

def equalf(a, b, eps=1e-5):
    if isinstance(a, Iterable) and isinstance(b, Iterable):
        return all([equalf(u, v) for u, v in zip(a, b)])
    elif not isinstance(a, Iterable) and not isinstance(b, Iterable):
        return abs(float(a) - float(b)) < eps
    else:
        return False

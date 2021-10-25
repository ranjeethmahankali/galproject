from os import path

def assetPath(filename):
    return path.join(path.dirname(path.dirname(path.realpath(__file__))),
                     "assets", filename)

def equalf(a, b, eps=1e-5):
    if isinstance(a, list) and isinstance(b, list):
        return all([equalf(u, v) for u, v in zip(a, b)])
    elif not isinstance(a, list) and not isinstance(b, list):
        return abs(float(a) - float(b)) < eps
    else:
        return False

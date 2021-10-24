from os import path

def assetPath(filename):
    return path.join(path.dirname(path.dirname(path.realpath(__file__))),
                     "assets", filename)

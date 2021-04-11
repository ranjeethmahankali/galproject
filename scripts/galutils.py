import os
import sys

def GetRootDir():
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def GetRelativePath(path: str):
    return os.path.join(GetRootDir(), path)


def AppendBinPath():
    binpath = GetRelativePath("build")
    print("Appending %s to the path" % binpath)
    sys.path.append(binpath) 
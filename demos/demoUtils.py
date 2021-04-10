import os


def GetRootDir():
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


if __name__=="__main__":
    print("Root directory: %s" % GetRootDir())

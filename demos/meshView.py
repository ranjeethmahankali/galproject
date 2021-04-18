import pygalfunc as pgf
import pygalview as pgv

relpath, = pgf.string("../assets/conference.obj")
path, = pgf.absPath(relpath)
mesh, = pgf.loadObjFile(path)
pgv.show(mesh)
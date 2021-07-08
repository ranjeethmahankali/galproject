import pygalfunc as pgf
import pygalview as pgv

relpath, = pgf.string("../assets/bunny_large.obj")
# relpath, = pgv.textField("Relative file path");
path, = pgf.absPath(relpath)
mesh, = pgf.loadObjFile(path)
box, = pgf.meshBbox(mesh)
cent, = pgf.meshCentroid(mesh)

print(pgf.read(cent))

pgv.show("mesh", mesh)
pgv.show("bounds", box)

"""Simply load the mesh and show some basic data about it."""
import pygalfunc as pgf
import pygalview as pgv

relpath = pgf.var_string("../../assets/bunny_large.obj")
# relpath = pgv.textField("Relative file path");
path = pgf.absPath(relpath)
mesh = pgf.loadTriangleMesh(path)
box = pgf.bounds(mesh)
pgv.show("mesh", mesh)
pgv.show("bounds", box)
pgv.show("centroid", pgf.centroid(mesh))

pgv.runCommands("""
perspective off
wireframe on
""")

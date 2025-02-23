"""Simply load the mesh and show some basic data about it."""
import pygalfunc as pgf
import pygalview as pgv

relpath = pgv.textField("relpath")
bunny = pgv.scale_mesh_simple(pgf.loadTriangleMesh(pgf.absPath(relpath)),
                              pgf.var_float(10.))
pgv.show("bunny", bunny)

pgv.runCommands("""
meshedges on
""")

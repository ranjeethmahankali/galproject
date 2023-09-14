"""Simply load the mesh and show some basic data about it."""
import pygalfunc as pgf
import pygalview as pgv

tmesh = pgf.loadTriangleMesh(
    pgf.absPath(pgf.var_string("../../assets/bunny_large.obj")))
pgv.show("bunny", tmesh)
pgv.show("bunnyBounds", pgf.bounds(tmesh))
pgv.show("bunnyCentroid", pgf.centroid(tmesh))
qmesh = pgf.scale(
    pgf.loadPolyMesh(pgf.absPath(pgf.var_string("../../assets/cane.obj"))),
    pgf.var_float(0.1))
pgv.show("cane", qmesh)

pgv.runCommands("""
meshedges on
""")

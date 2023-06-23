"""Mesh decimation."""
import pygalfunc as pgf
import pygalview as pgv

# relpath = pgf.var_string("../../assets/bunny.obj")
# original = pgf.loadTriangleMesh(pgf.var_string("/home/rnjth94/downloads/deer.obj"))
# original = pgf.loadTriangleMesh(
#     pgf.var_string("/home/rnjth94/downloads/helmet.obj"))
original = pgf.loadTriangleMesh(
    pgf.var_string("/home/rnjth94/downloads/quad-periodic-beam-lattice.obj"))
# original = pgf.loadTriangleMesh(
#     pgf.var_string("/home/rnjth94/downloads/quadbox.obj"))
pgv.show("original", original)
numcollapses = pgv.slideri32("nCollapses", 1, 100000, 10)
decimated = pgf.decimate(original, numcollapses)
pgv.show("decimated", decimated)
pgv.runCommands("""
meshedges on
""")

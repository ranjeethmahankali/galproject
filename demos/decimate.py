"""Mesh decimation."""
import pygalfunc as pgf
import pygalview as pgv

original = pgf.loadTriangleMesh(
    pgf.absPath(pgf.var_string("../../assets/bunny_large.obj")))
# pgv.show("original", original)
numcollapses = pgv.slideri32("nCollapses", 1, 34800, 10)
decimated = pgf.decimate(original, numcollapses)
pgv.show("decimated", decimated)
pgv.runCommands("""
meshedges on
""")

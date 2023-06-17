"""Mesh decimation."""
import pygalfunc as pgf
import pygalview as pgv

relpath = pgf.var_string("../../assets/bunny.obj")
original = pgf.loadObjFile(pgf.absPath(relpath))
pgv.show("original", original)
numcollapses = pgv.slideri32("nCollapses", 1, 1000, 500)
decimated = pgf.decimate(original, numcollapses)
pgv.show("decimated", decimated)
pgv.runCommands("""
meshedges on
""")

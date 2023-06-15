"""Mesh decimation."""
import pygalfunc as pgf
import pygalview as pgv

relpath = pgf.var_string("../../assets/bunny_large.obj")
original = pgf.loadObjFile(pgf.absPath(relpath))
meshes = pgf.decimateWithHistory(original)
pgv.show("original", original)
index = pgv.slideri32("Index", 0, 100, 0)
imesh = pgf.listItem(
    meshes, pgf.min(index, pgf.sub(pgf.listLength(meshes), pgf.var_int(1))))
pgv.show("Intermediate", imesh)

"""Test the cgal parametrization."""
import pygalfunc as pgf
import pygalview as pgv

mesh = pgf.loadTriangleMesh(
    pgf.absPath(
        pgf.var_string("/home/rnjth94/buffer/parametrization/manifold1.obj")))
pgv.show("mesh", mesh)

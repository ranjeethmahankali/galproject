"""Simply load the mesh and show some basic data about it."""
import pygalfunc as pgf
import pygalview as pgv

mesh = pgf.loadTriangleMesh(
    pgf.absPath(
        pgf.var_string(
            "/home/rnjth94/buffer/parametrization/surface_lattice.obj")))

# mesh = pgf.loadTriangleMesh(
#     pgf.absPath(pgf.var_string("../../assets/bunny.obj")))

numcollapses = pgv.slideri32("nCollapses", 1, 1500000, 10)
simpler = pgf.simplify(mesh, numcollapses)
numfaces = pgf.numFaces(simpler)
numverts = pgf.numVertices(simpler)

pgv.show("original", mesh)
pgv.show("simplified", simpler)
pgv.show("vertices", simpler)
pgv.print("num_vertices", numverts)
pgv.print("num_faces", numfaces)
pgv.print("original_num_edges", pgf.numEdges(mesh))

pgv.runCommands("""
meshedges on
""")

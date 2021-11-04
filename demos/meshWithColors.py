import pygalfunc as pgf
import pygalview as pgv


pgv.set2dMode(True)

pt = pgf.var_vec3((0., 0., 0.))
norm = pgf.var_vec3((0., 0., 1.))
plane = pgf.plane(pt, norm)
minpt = pgf.var_vec3((-.5, -.5, 0.))
maxpt = pgf.var_vec3((.5, .5, 0.))
box2 = pgf.box2(pgf.var_vec2((-.5, -.5)), pgf.var_vec2((.5, .5)))
box3 = pgf.box3(minpt, maxpt)
npts = pgv.slideri32("Point count", 5, 50, 25)
cloud = pgf.randomPointsInBox(box3, npts)

edgeLen = pgf.var_float(0.01)
rect = pgf.rectangleMesh(plane, box2, edgeLen)

distances = pgf.distance(pgf.graft(pgf.vertices(rect)), cloud)
sortedDists = pgf.sort(distances, distances)
maxDist = pgf.listItem(sortedDists, pgf.sub(pgf.listLength(sortedDists), pgf.var_int(1)))

scheme = pgf.var_vec3([
    (0., 0., 1.),
    (0., 1., 0.),
    (1., 1., 0.),
    (1., 0., 0.)
])
colors = pgf.mapValueToColor(maxDist, pgf.var_vec2((.5, 1.2)), scheme)
colored = pgf.meshWithVertexColors(rect, colors)

circ, *_ = pgf.boundingCircle(cloud)

# pgv.show("rectangle", rect)
pgv.show("cloud", cloud)
pgv.show("colored", colored)
pgv.show("circle", circ)

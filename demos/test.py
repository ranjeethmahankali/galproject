import pygalfunc as pgf
import pygalview as pgv

plane = pgf.plane(pgf.var_vec3((0., 0., 0.)), pgf.var_vec3((0., 0., 1.)))
box = pgf.box2(pgf.var_vec2((0., 0.)), pgf.var_vec2((15., 12.)))
edgeLn = pgf.var_float(1.)

rect = pgf.rectangleMesh(plane, box, edgeLn)

pgv.show("rect", rect)

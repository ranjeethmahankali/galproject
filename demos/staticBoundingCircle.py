import pygalfunc as pgf
import pygalview as pgv
import random

random.seed(42)
pypoints = [[random.uniform(-1.0, 1.0) for i in range(3)] for j in range(20)]

pgv.set2dMode(True)
pts, = pgf.listvec3(pypoints)
pgv.print("My points", pts)
cloud, = pgf.pointCloud3d(pts)
circ, = pgf.boundingCircle(cloud)
print(cloud)
pgv.show(cloud)
pgv.show(circ)

# temp = [42.42, 4.242, .4242, 424.2]
# nums, = pgf.listf32(temp)
# print(nums)
# pgv.print("MyList", nums)
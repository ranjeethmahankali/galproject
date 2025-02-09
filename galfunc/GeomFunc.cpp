#include <algorithm>

#include <Box.h>
#include <ConvexHull.h>
#include <Functions.h>

namespace gal {
namespace func {

GAL_FUNC(vec3,  // NOLINT
         "Creates a 3D vector from coordinates",
         ((float, x, "x coordinate"),
          (float, y, "y coordinate"),
          (float, z, "z coordinate")),
         ((glm::vec3, vector, "3D vector")))
{
  vector.x = x;
  vector.y = y;
  vector.z = z;
}

GAL_FUNC(vec2,  // NOLINT
         "Creates a 2D vector from coordinates",
         ((float, x, "x coordinate"), (float, y, "y coordinate")),
         ((glm::vec2, vector, "2D vector")))
{
  vector.x = x;
  vector.y = y;
}

GAL_FUNC(vec3FromVec2,  // NOLINT
         "Creates a 2D vector from coordinates",
         ((glm::vec2, v2, "2d vector")),
         ((glm::vec3, v3, "3D vector")))
{
  v3.x = v2.x;
  v3.y = v2.y;
  v3.z = 0.f;
}

GAL_FUNC(vec2FromVec3,  // NOLINT
         "Creates a 2D vector from coordinates",
         ((glm::vec3, v3, "3d vector")),
         ((glm::vec2, v2, "2d vector")))
{
  v2.x = v3.x;
  v2.y = v3.y;
}

GAL_FUNC(vec3Coords,  // NOLINT
         "Gets the coordinates of the vector",
         ((glm::vec3, v, "Vector")),
         ((float, x, "x coordinate"),
          (float, y, "y coordinate"),
          (float, z, "z coordinate")))
{
  x = v.x;
  y = v.y;
  z = v.z;
}

GAL_FUNC(vec2Coords,  // NOLINT
         "Gets the coordinates of the vector",
         ((glm::vec2, v, "Vector")),
         ((float, x, "x coordinate"), (float, y, "y coordinate")))
{
  x = v.x;
  y = v.y;
}

GAL_FUNC(plane,  // NOLINT
         "Creates a plane with the given point and normal",
         ((glm::vec3, point, "Point"), (glm::vec3, normal, "Normal")),
         ((gal::Plane, plane, "The plane")))
{
  plane.origin(point);
  plane.normal(normal);
}

GAL_FUNC(box3,  // NOLINT
         "Creates a 3d box with the two given points",
         ((glm::vec3, min, "min point"), (glm::vec3, max, "max point")),
         ((gal::Box3, box, "Box")))
{
  box.min = min;
  box.max = max;
}

GAL_FUNC(box2,  // NOLINT
         "Creates a 2d box with the two given points",
         ((glm::vec2, min, "min point"), (glm::vec2, max, "max point")),
         ((gal::Box2, box, "Box")))
{
  box.min = min;
  box.max = max;
}

GAL_FUNC(randomPointsInBox,  // NOLINT
         "Creates a random point cloud with points inside the given box",
         ((gal::Box3, box, "Box to sample from"),
          (int32_t, numPoints, "Number of points to sample")),
         (((data::WriteView<glm::vec3, 1>), points, "Point cloud")))
{
  size_t nPts = size_t(numPoints);
  points.reserve(nPts);
  box.randomPoints(nPts, std::back_inserter(points));
}

GAL_FUNC(convexHullFromPoints,  // NOLINT
         "Creates a convex hull from the given point cloud",
         (((data::ReadView<glm::vec3, 1>), points, "Point cloud")),
         ((gal::TriMesh, hull, "Convex hull")))
{
  hull = std::move(gal::ConvexHull(points.begin(), points.end()).toMesh());
}

GAL_FUNC(pointCloud3d,  // NOLINT
         "Creates a point cloud from the list of points",
         (((data::ReadView<glm::vec3, 1>), points, "points")),
         ((gal::PointCloud<3>, cloud, "Point cloud")))
{
  cloud.resize(points.size());
  std::copy(points.begin(), points.end(), cloud.begin());
}

GAL_FUNC(distance3,  // NOLINT
         "Gets the distance betwen the two points",
         ((glm::vec3, a, "first point"), (glm::vec3, b, "second point")),
         ((float, dist, "Distance")))
{
  dist = glm::distance(a, b);
}

GAL_FUNC(distance2,  // NOLINT
         "Gets the distance betwen the two points",
         ((glm::vec2, a, "first point"), (glm::vec2, b, "second point")),
         ((float, dist, "Distance")))
{
  dist = glm::distance(a, b);
}

GAL_FUNC(box3Points,  // NOLINT
         "Gets the min and max points of the box",
         ((gal::Box3, box, "The box.")),
         ((glm::vec3, min, "Min point"), (glm::vec3, max, "Max point")))
{
  min = box.min;
  max = box.max;
}

GAL_FUNC(box2Points,  // NOLINT
         "Gets the min and max points of the box",
         ((gal::Box2, box, "The box.")),
         ((glm::vec2, min, "Min point"), (glm::vec2, max, "Max point")))
{
  min = box.min;
  max = box.max;
}

void bind_GeomFunc(py::module& module)
{
  GAL_FN_BIND(vec3, module);
  GAL_FN_BIND(vec2, module);
  GAL_FN_BIND(plane, module);
  GAL_FN_BIND(box3, module);
  GAL_FN_BIND(box2, module);
  GAL_FN_BIND(randomPointsInBox, module);
  GAL_FN_BIND(convexHullFromPoints, module);
  GAL_FN_BIND(pointCloud3d, module);

  // TODO: Handle these with generic converters later.
  GAL_FN_BIND(vec3FromVec2, module);
  GAL_FN_BIND(vec2FromVec3, module);

  GAL_FN_BIND_OVERLOADS(module, coords, vec3Coords, vec2Coords);
  GAL_FN_BIND_OVERLOADS(module, boxPoints, box2Points, box3Points);
  GAL_FN_BIND_OVERLOADS(module, distance, distance2, distance3);
}

}  // namespace func
}  // namespace gal

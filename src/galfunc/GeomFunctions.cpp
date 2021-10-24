#include <algorithm>

#include <galcore/ConvexHull.h>
#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(vec3,
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

GAL_FUNC(vec2,
         "Creates a 2D vector from coordinates",
         ((float, x, "x coordinate"), (float, y, "y coordinate")),
         ((glm::vec2, vector, "2D vector")))
{
  vector.x = x;
  vector.y = y;
}

GAL_FUNC(vec3FromVec2,
         "Creates a 2D vector from coordinates",
         ((glm::vec2, v2, "2d vector")),
         ((glm::vec3, v3, "3D vector")))
{
  v3.x = v2.x;
  v3.y = v2.y;
  v3.z = 0.f;
}

GAL_FUNC(vec2FromVec3,
         "Creates a 2D vector from coordinates",
         ((glm::vec3, v3, "3d vector")),
         ((glm::vec2, v2, "2d vector")))
{
  v2.x = v3.x;
  v2.y = v3.y;
}

GAL_FUNC(vec3Coords,
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

GAL_FUNC(vec2Coords,
         "Gets the coordinates of the vector",
         ((glm::vec2, v, "Vector")),
         ((float, x, "x coordinate"), (float, y, "y coordinate")))
{
  x = v.x;
  y = v.y;
}

GAL_FUNC(plane,
         "Creates a plane with the given point and normal",
         ((glm::vec3, point, "Point"), (glm::vec3, normal, "Normal")),
         ((gal::Plane, plane, "The plane")))
{
  plane.origin(point);
  plane.normal(normal);
}

GAL_FUNC(box3,
         "Creates a 3d box with the two given points",
         ((glm::vec3, min, "min point"), (glm::vec3, max, "max point")),
         ((gal::Box3, box, "Box")))
{
  box.min = min;
  box.max = max;
}

GAL_FUNC(box2,
         "Creates a 2d box with the two given points",
         ((glm::vec2, min, "min point"), (glm::vec2, max, "max point")),
         ((gal::Box2, box, "Box")))
{
  box.min = min;
  box.max = max;
}

GAL_FUNC(randomPointsInBox,
         "Creates a random point cloud with points inside the given box",
         ((gal::Box3, box, "Box to sample from"),
          (int32_t, numPoints, "Number of points to sample")),
         (((data::WriteView<glm::vec3, 1>), points, "Point cloud")))
{
  size_t nPts = size_t(numPoints);
  points.reserve(nPts);
  box.randomPoints(nPts, std::back_inserter(points));
}

GAL_FUNC(convexHullFromPoints,
         "Creates a convex hull from the given point cloud",
         (((data::ReadView<glm::vec3, 1>), points, "Point cloud")),
         ((gal::Mesh, hull, "Convex hull")))
{
  hull = std::move(gal::ConvexHull(points.begin(), points.end()).toMesh());
}

GAL_FUNC(pointCloud3d,
         "Creates a point cloud from the list of points",
         (((data::ReadView<glm::vec3, 1>), points, "points")),
         ((gal::PointCloud, cloud, "Point cloud")))
{
  cloud.resize(points.size());
  std::copy(points.begin(), points.end(), cloud.begin());
}

GAL_FUNC(distance,
         "Gets the distance betwen the two points",
         ((glm::vec3, a, "first point"), (glm::vec3, b, "second point")),
         ((float, dist, "Distance")))
{
  dist = glm::distance(a, b);
}

void bind_GeomFunctions()
{
  GAL_FN_BIND(vec3,
              vec2,
              plane,
              box3,
              box2,
              randomPointsInBox,
              convexHullFromPoints,
              pointCloud3d,
              distance);

  // TODO: Handle these with generic converters later.
  GAL_FN_BIND(vec3FromVec2, vec2FromVec3);

  GAL_FN_BIND_OVERLOADS(coords, vec3Coords, vec2Coords);
}

}  // namespace func
}  // namespace gal

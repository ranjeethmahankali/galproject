#include <algorithm>

#include <galcore/ConvexHull.h>
#include <galfunc/GeomFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(vec3, ((float, x), (float, y), (float, z)), ((glm::vec3, vector)))
{
  vector.x = x;
  vector.y = y;
  vector.z = z;
};

GAL_FUNC_DEFN(vec2, ((float, x), (float, y)), ((glm::vec2, vector)))
{
  vector.x = x;
  vector.y = y;
}

GAL_FUNC_DEFN(vec3FromVec2, ((glm::vec2, v2)), ((glm::vec3, v3)))
{
  v3.x = v2.x;
  v3.y = v2.y;
  v3.z = 0.f;
}

GAL_FUNC_DEFN(vec2FromVec3, ((glm::vec3, v3)), ((glm::vec2, v2)))
{
  v2.x = v3.x;
  v2.y = v3.y;
}

GAL_FUNC_DEFN(vec3Coords, ((glm::vec3, v)), ((float, x), (float, y), (float, z)))
{
  x = v.x;
  y = v.y;
  z = v.z;
}

GAL_FUNC_DEFN(vec2Coords, ((glm::vec2, v)), ((float, x), (float, y)))
{
  x = v.x;
  y = v.y;
}

GAL_FUNC_DEFN(plane, ((glm::vec3, point), (glm::vec3, normal)), ((gal::Plane, plane)))
{
  plane.origin(point);
  plane.normal(normal);
};

GAL_FUNC_DEFN(box3, ((glm::vec3, min), (glm::vec3, max)), ((gal::Box3, box)))
{
  box.min = min;
  box.max = max;
};

GAL_FUNC_DEFN(box2, ((glm::vec2, min), (glm::vec2, max)), ((gal::Box2, box)))
{
  box.min = min;
  box.max = max;
};

GAL_FUNC_DEFN(randomPointsInBox,
              ((gal::Box3, box), (int32_t, numPoints)),
              (((data::WriteView<glm::vec3, 1>), points)))
{
  size_t nPts = size_t(numPoints);
  points.reserve(nPts);
  box.randomPoints(nPts, std::back_inserter(points));
};

GAL_FUNC_DEFN(convexHullFromPoints,
              (((data::ReadView<glm::vec3, 1>), points)),
              ((gal::Mesh, hull)))
{
  hull = std::move(gal::ConvexHull(points.begin(), points.end()).toMesh());
}

GAL_FUNC_DEFN(pointCloud3d,
              (((data::ReadView<glm::vec3, 1>), points)),
              ((gal::PointCloud, cloud)))
{
  cloud.resize(points.size());
  std::copy(points.begin(), points.end(), cloud.begin());
}

GAL_FUNC_DEFN(distance, ((glm::vec3, a), (glm::vec3, b)), ((float, dist)))
{
  dist = glm::distance(a, b);
}

}  // namespace func
}  // namespace gal

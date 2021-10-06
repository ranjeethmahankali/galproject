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

GAL_FUNC_DEFN(randomPointCloudFromBox,
              ((gal::Box3, box), (int32_t, numPoints)),
              ((gal::PointCloud, cloud)))
{
  size_t nPts = size_t(numPoints);
  cloud.clear();
  cloud.reserve(nPts);
  box.randomPoints(nPts, std::back_inserter(cloud));
};

GAL_FUNC_DEFN(pointCloudConvexHull, ((gal::PointCloud, cloud)), ((gal::Mesh, hull)))
{
  hull = std::move(gal::ConvexHull(cloud.begin(), cloud.end()).toMesh());
}

GAL_FUNC_DEFN(listVec3FromPointCloud,
              ((gal::PointCloud, cloud)),
              ((std::vector<glm::vec3>, pts)))
{
  pts.resize(cloud.size());
  std::copy(cloud.begin(), cloud.end(), pts.begin());
}

GAL_FUNC_DEFN(pointCloud3d,
              ((std::vector<glm::vec3>, points)),
              ((gal::PointCloud, cloud)))
{
  cloud.resize(points.size());
  std::copy(points.begin(), points.end(), cloud.begin());
}

GAL_FUNC_DEFN(distance, ((glm::vec3, a), (glm::vec3, b)), ((float, dist)))
{
  dist = glm::distance(a, b);
}

GAL_FUNC_DEFN(pointCloudFarthestPt,
              ((gal::PointCloud, cloud), (glm::vec3, pt)),
              ((glm::vec3, farthest)))
{
  farthest = *(std::max_element(
    cloud.begin(), cloud.end(), [&pt](const glm::vec3& a, const glm::vec3& b) {
      return glm::distance2(a, pt) < glm::distance2(b, pt);
    }));
}

}  // namespace func
}  // namespace gal

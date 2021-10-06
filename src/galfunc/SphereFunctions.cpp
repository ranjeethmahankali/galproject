#include <galfunc/SphereFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(sphere, ((glm::vec3, center), (float, radius)), ((gal::Sphere, sphere)))
{
  sphere.center = center;
  sphere.radius = radius;
};

GAL_FUNC_DEFN(boundingSphere,
              ((gal::PointCloud, points)),
              ((gal::Sphere, sphere), (glm::vec3, center), (float, radius)))
{
  sphere = gal::Sphere::minBoundingSphere(points);
  center = sphere.center;
  radius = sphere.radius;
};

}  // namespace func
}  // namespace gal

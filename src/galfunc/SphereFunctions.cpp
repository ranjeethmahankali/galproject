#include <galfunc/SphereFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(sphere,
              2,
              1,
              "Creates a new sphere",
              ((glm::vec3, center, "Center"), (float, radius, "Radius")),
              ((gal::Sphere, sphere, "Sphere")))
{
  sphere->center = *center;
  sphere->radius = *radius;
};

GAL_FUNC_DEFN(boundingSphere,
              1,
              3,
              "Creates a minimum bounding sphere for the given points.",
              ((gal::PointCloud, points, "Points")),
              ((gal::Sphere, sphere, "Bounding sphere"),
               (glm::vec3, center, "Center of the sphere"),
               (float, radius, "Radius of the sphere")))
{
  *sphere = gal::Sphere::minBoundingSphere(*points);
  *center = sphere->center;
  *radius = sphere->radius;
};

}  // namespace func
}  // namespace gal

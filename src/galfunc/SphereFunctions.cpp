#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(sphere,
              "Creates a new sphere",
              ((glm::vec3, center, "Center"), (float, radius, "Radius")),
              ((gal::Sphere, sphere, "Sphere")))
{
  sphere.center = center;
  sphere.radius = radius;
}

GAL_FUNC_DECL(boundingSphere,
              "Creates a minimum bounding sphere for the given points.",
              (((data::ReadView<glm::vec3, 1>), points, "Points")),
              ((gal::Sphere, sphere, "Bounding sphere"),
               (glm::vec3, center, "Center of the sphere"),
               (float, radius, "Radius of the sphere")))
{
  sphere = gal::Sphere::minBoundingSphere(points.data(), points.size());
  center = sphere.center;
  radius = sphere.radius;
}

void bind_SphereFunctions()
{
  bind_sphere();
  bind_boundingSphere();
}

}  // namespace func
}  // namespace gal

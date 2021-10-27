#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(sphere,
         "Creates a new sphere",
         ((glm::vec3, center, "Center"), (float, radius, "Radius")),
         ((gal::Sphere, sphere, "Sphere")))
{
  sphere.center = center;
  sphere.radius = radius;
}

GAL_FUNC(boundingSphere,
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

GAL_FUNC(bounds,
         "Bounding box of the sphere",
         ((gal::Sphere, s, "Sphere")),
         ((gal::Box3, bbox, "Bounding box")))
{
  bbox = s.bounds();
}

void bind_SphereFunctions()
{
  GAL_FN_BIND(bounds, sphere, boundingSphere);
}

}  // namespace func
}  // namespace gal

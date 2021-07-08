#include <galfunc/SphereFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(((gal::Sphere, sphere, "Sphere")),
              sphere,
              true,
              2,
              "Creates a new sphere",
              (glm::vec3, center, "Center"),
              (float, radius, "Radius"))
{
  return std::make_tuple(std::make_shared<gal::Sphere>(gal::Sphere {*center, *radius}));
};

GAL_FUNC_DEFN(((gal::Sphere, sphere, "Bounding sphere"),
               (glm::vec3, center, "Center of the sphere"),
               (float, radius, "Radius of the sphere")),
              boundingSphere,
              true,
              1,
              "Creates a minimum bounding sphere for the given points.",
              (gal::PointCloud, points, "Points"))
{
  auto      sphere = gal::Sphere::minBoundingSphere(*points);
  glm::vec3 center = sphere.center;
  float     radius = sphere.radius;
  return std::make_tuple(std::make_shared<gal::Sphere>(std::move(sphere)),
                         std::make_shared<glm::vec3>(std::move(center)),
                         std::make_shared<float>(radius));
};

}  // namespace func
}  // namespace gal
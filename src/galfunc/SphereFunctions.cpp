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

GAL_FUNC_DEFN(((gal::Sphere, sphere, "Bounding sphere")),
              boundingSphere,
              true,
              1,
              "Creates a minimum bounding sphere for the given points.",
              (gal::PointCloud, points, "Points"))
{
  return std::make_tuple(
    std::make_shared<gal::Sphere>(gal::Sphere::minBoundingSphere(*points)));
};

}  // namespace func
}  // namespace gal
#include <galcore/Sphere.h>

namespace gal {

Box3 Sphere::bounds() const
{
  float     r = std::abs(radius);
  glm::vec3 rv {r, r, r};
  return Box3(center - rv, center + rv);
}

}  // namespace gal
#pragma once
#include <glm/glm.hpp>
#include <galcore/Box.h>

namespace gal {
  
struct Sphere
{
  glm::vec3 center;
  float radius;

  Box3 bounds() const;
};

}  // namespace gal
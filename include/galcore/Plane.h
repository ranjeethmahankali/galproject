#pragma once
#include <glm/glm.hpp>

namespace gal {
  
struct Plane
{
  glm::vec3 origin;
  glm::vec3 normal;
};

}  // namespace gal
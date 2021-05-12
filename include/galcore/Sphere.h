#pragma once
#include <galcore/Box.h>
#include <glm/glm.hpp>

namespace gal {

struct Sphere
{
  glm::vec3 center;
  float     radius;

  Box3 bounds() const;
};

template<>
struct Serial<Sphere> : public std::true_type
{
  static Sphere deserialize(Bytes& bytes)
  {
    Sphere sp;
    bytes >> sp.center >> sp.radius;
    return sp;
  }
  static Bytes serialize(const Sphere& sp)
  {
    Bytes bytes;
    bytes << sp.center << sp.radius;
    return bytes;
  }
};

}  // namespace gal
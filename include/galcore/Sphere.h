#pragma once
#include <galcore/Box.h>
#include <glm/glm.hpp>

namespace gal {

struct Sphere
{
  glm::vec3 center = glm::vec3 {0.f, 0.f, 0.f};
  float     radius = 0.f;

  Sphere() = default;
  Sphere(const glm::vec3&, float);

  Box3 bounds() const;

  bool contains(const glm::vec3& pt) const;

  static Sphere createCircumsphere(const glm::vec3& a,
                                   const glm::vec3& b,
                                   const glm::vec3& c,
                                   const glm::vec3& d);
  static Sphere createFromDiameter(const glm::vec3& a, const glm::vec3& b);
  static Sphere minBoundingSphere(const std::vector<glm::vec3>& points);
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
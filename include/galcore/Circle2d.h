#pragma once
#include <galcore/Box.h>
#include <galcore/Serialization.h>
#include <glm/glm.hpp>

namespace gal {

class Circle2d
{
public:
  Circle2d() = default;
  Circle2d(const glm::vec2& center, float radius);

  const glm::vec2& center() const;
  float            radius() const;
  bool             contains(const glm::vec2&) const;

  Box2 bounds() const;

  static Circle2d createCircumcircle(const glm::vec2& a,
                                     const glm::vec2& b,
                                     const glm::vec2& c);
  static Circle2d createFromDiameter(const glm::vec2& a, const glm::vec2& b);
  static Circle2d minBoundingCircle(const std::vector<glm::vec2>& points);

private:
  glm::vec2 mCenter = {0.f, 0.f};
  float     mRadius = 0.f;
};

template<>
struct Serial<Circle2d> : public std::true_type
{
  static Circle2d deserialize(Bytes& bytes)
  {
    glm::vec2 center;
    float     radius;
    bytes >> center >> radius;
    return Circle2d(center, radius);
  }

  static Bytes serialize(const Circle2d& cloud)
  {
    Bytes bytes;
    bytes << cloud.center() << cloud.radius();
    return bytes;
  }
};

}  // namespace gal
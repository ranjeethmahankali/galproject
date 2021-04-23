#pragma once
#include <glm/glm.hpp>
#include <galcore/Box.h>

namespace gal {

class Circle2d
{
public:
  Circle2d(const glm::vec2& center, float radius);

  const glm::vec2& center() const;
  float            radius() const;
  bool             contains(const glm::vec2&) const;

  Box2 bounds() const;

  static Circle2d createCircumcircle(const glm::vec2& a,
                                     const glm::vec2& b,
                                     const glm::vec2& c);
  static Circle2d createFromDiameter(const glm::vec2& a, const glm::vec2& b);
  static Circle2d minBoundingCircle(const glm::vec2* pts, size_t n);

private:
  glm::vec2 mCenter;
  float     mRadius;
};

}  // namespace gal
#include <galcore/Circle2d.h>
#include <galcore/DebugProfile.h>
#include <algorithm>
#include <glm/gtx/norm.hpp>

namespace gal {

Circle2d::Circle2d(const glm::vec2& center, float radius)
    : mCenter(center)
    , mRadius(std::abs(radius)) {};

const glm::vec2& Circle2d::center() const
{
  return mCenter;
};

float Circle2d::radius() const
{
  return mRadius;
};

bool Circle2d::contains(const glm::vec2& pt) const
{
  return glm::distance2(mCenter, pt) <= (mRadius * mRadius);
};

Box2 Circle2d::bounds() const
{
  glm::vec2 r {mRadius, mRadius};
  return Box2(mCenter - r, mCenter + r);
}

Circle2d Circle2d::createCircumcircle(const glm::vec2& a,
                                      const glm::vec2& b,
                                      const glm::vec2& c)
{
  GALSCOPE(__func__);
  float a2 = glm::length2(a);
  float b2 = glm::length2(b);
  float c2 = glm::length2(c);

  float adet = 1.0f / glm::determinant(
                        glm::mat3 {{a.x, a.y, 1.f}, {b.x, b.y, 1.f}, {c.x, c.y, 1.f}});

  glm::vec2 center =
    adet * 0.5f *
    glm::vec2 {
      glm::determinant(glm::mat3 {{a2, a.y, 1.f}, {b2, b.y, 1.f}, {c2, c.y, 1.f}}),
      glm::determinant(glm::mat3 {{a.x, a2, 1.f}, {b.x, b2, 1.f}, {c.x, c2, 1.f}})};

  return Circle2d(center, glm::distance(center, a));
};

Circle2d Circle2d::createFromDiameter(const glm::vec2& a, const glm::vec2& b)
{
  glm::vec2 center = 0.5f * (a + b);
  return Circle2d(center, glm::distance(center, a));
};

static void minBoundingCircleImpl(Circle2d&        circ,
                                  const glm::vec2* begin,
                                  const glm::vec2* end,
                                  const glm::vec2* pin1 = nullptr,
                                  const glm::vec2* pin2 = nullptr)
{
  auto current = begin;
  if (pin1 && pin2) {
    circ = Circle2d::createFromDiameter(*pin1, *pin2);
    GALWATCH(circ);
  }
  else if (pin1) {
    circ = Circle2d::createFromDiameter(*(current++), *pin1);
    GALWATCH(circ);
  }
  else {
    circ = Circle2d::createFromDiameter(*current, *(current + 1));
    GALWATCH(circ);
    current += 2;
  }

  while (current != end) {
    if (!circ.contains(*current)) {
      if (pin1 && pin2) {
        circ = Circle2d::createCircumcircle(*pin1, *pin2, *current);
        GALWATCH(circ);
      }
      else if (pin1) {
        minBoundingCircleImpl(circ, begin, current, pin1, current);
      }
      else {
        minBoundingCircleImpl(circ, begin, current, current);
      }
    }
    current++;
  }
}

Circle2d Circle2d::minBoundingCircle(const std::vector<glm::vec2>& points)
{
  GALSCOPE(__func__);
  if (points.size() < 2) {
    throw "Cannot compute circle";
  }

  Circle2d circ;
  minBoundingCircleImpl(circ, points.data(), points.data() + points.size());
  GALWATCH(circ);
  return circ;
};

}  // namespace gal
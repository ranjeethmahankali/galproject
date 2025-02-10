#include <Circle2d.h>

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

void Circle2d::center(const glm::vec2& newCenter)
{
  mCenter = newCenter;
}

float Circle2d::radius() const
{
  return mRadius;
};

void Circle2d::radius(float newRadius)
{
  mRadius = newRadius;
}

bool Circle2d::contains(const glm::vec2& pt, float tolerance) const
{
  return glm::distance(mCenter, pt) <= mRadius + tolerance;
};

float Circle2d::area() const
{
  return M_PI * mRadius * mRadius;
}

float Circle2d::perimeter() const
{
  static constexpr float s2Pi = 2.f * M_PI;
  return s2Pi * mRadius;
}

Box2 Circle2d::bounds() const
{
  glm::vec2 r {mRadius, mRadius};
  return Box2(mCenter - r, mCenter + r);
}

Circle2d Circle2d::createCircumcircle(const glm::vec2& a,
                                      const glm::vec2& b,
                                      const glm::vec2& c)
{
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
  }
  else if (pin1) {
    circ = Circle2d::createFromDiameter(*(current++), *pin1);
  }
  else {
    circ = Circle2d::createFromDiameter(*current, *(current + 1));
    current += 2;
  }

  while (current != end) {
    if (!circ.contains(*current)) {
      if (pin1 && pin2) {
        circ = Circle2d::createCircumcircle(*pin1, *pin2, *current);
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

Circle2d Circle2d::minBoundingCircle(const glm::vec2* pts, size_t npts)
{
  if (npts < 2) {
    throw "Cannot compute circle";
  }

  Circle2d circ;
  minBoundingCircleImpl(circ, pts, pts + npts);
  return circ;
};

}  // namespace gal

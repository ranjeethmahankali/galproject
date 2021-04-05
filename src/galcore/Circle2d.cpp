#include <galcore/Circle2d.h>
#include <algorithm>
#include <glm/gtx/norm.hpp>

namespace gal {

Circle2d::Circle2d(const glm::vec2& center, float radius)
    : mCenter(center)
    , mRadius(radius) {};

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

static Circle2d minBoundingCircle2Pt(const glm::vec2* pts,
                                     size_t           n,
                                     const glm::vec2& p,
                                     const glm::vec2& q)
{
  Circle2d circ = Circle2d::createFromDiameter(p, q);
  for (size_t i = 0; i < n; i++) {
    if (!circ.contains(pts[i])) {
      circ = Circle2d::createCircumcircle(pts[i], p, q);
    }
  }

  return circ;
};

static Circle2d minBoundingCircle1Pt(const glm::vec2* pts, size_t n, const glm::vec2& pt)
{
  Circle2d circ = Circle2d::createFromDiameter(pts[0], pt);
  for (size_t i = 1; i < n; i++) {
    if (!circ.contains(pts[i])) {
      circ = minBoundingCircle2Pt(pts, i, pts[i], pt);
    }
  }
  return circ;
};

Circle2d Circle2d::minBoundingCircle(const glm::vec2* pts, size_t n)
{
  if (n < 2) {
    throw "Cannot compute circle";
  }
  if (n == 3) {
    return Circle2d::createCircumcircle(pts[0], pts[1], pts[2]);
  }

  Circle2d circ = createFromDiameter(pts[0], pts[1]);
  if (n == 2) {
    return circ;
  }
  for (size_t i = 2; i < n; i++) {
    if (!circ.contains(pts[i])) {
      circ = minBoundingCircle1Pt(pts, i, pts[i]);
    }
  }
  return circ;
};

}  // namespace gal
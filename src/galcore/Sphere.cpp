#include <galcore/DebugProfile.h>
#include <galcore/Sphere.h>

namespace gal {

Box3 Sphere::bounds() const
{
  float     r = std::abs(radius);
  glm::vec3 rv {r, r, r};
  return Box3(center - rv, center + rv);
}

Sphere::Sphere(const glm::vec3& c, float r)
    : center(c)
    , radius(r)
{}

bool Sphere::contains(const glm::vec3& pt, float tolerance) const
{
  return glm::distance(center, pt) <= radius + tolerance;
}

Sphere Sphere::createCircumsphere(const glm::vec3& a,
                                  const glm::vec3& b,
                                  const glm::vec3& c,
                                  const glm::vec3& d)
{
  float a2 = glm::length2(a);
  float b2 = glm::length2(b);
  float c2 = glm::length2(c);
  float d2 = glm::length2(d);

  float adet = 1.0f / glm::determinant(glm::mat4 {{a.x, a.y, a.z, 1.f},
                                                  {b.x, b.y, b.z, 1.f},
                                                  {c.x, c.y, c.z, 1.f},
                                                  {d.x, d.y, d.z, 1.f}});

  glm::vec3 center = adet * 0.5f *
                     glm::vec3 {glm::determinant(glm::mat4 {{a2, a.y, a.z, 1.f},
                                                            {b2, b.y, b.z, 1.f},
                                                            {c2, c.y, c.z, 1.f},
                                                            {d2, d.y, d.z, 1.f}}),
                                glm::determinant(glm::mat4 {{a.x, a2, a.z, 1.f},
                                                            {b.x, b2, b.z, 1.f},
                                                            {c.x, c2, c.z, 1.f},
                                                            {d.x, d2, d.z, 1.f}}),
                                glm::determinant(glm::mat4 {
                                  {a.x, a.y, a2, 1.f},
                                  {b.x, b.y, b2, 1.f},
                                  {c.x, c.y, c2, 1.f},
                                  {d.x, d.y, d2, 1.f},
                                })};

  return Sphere(center, glm::distance(center, a));
}

Sphere Sphere::createFromDiameter(const glm::vec3& a, const glm::vec3& b)
{
  auto center = (a + b) * 0.5f;
  return Sphere(center, glm::distance(a, center));
}

static Sphere triangleCircumsphere(const glm::vec3& a,
                                   const glm::vec3& b,
                                   const glm::vec3& c)
{
  auto ca  = c - a;
  auto ba  = b - a;
  auto crs = glm::cross(ba, ca);

  glm::vec3 rvec =
    (glm::length2(ca) * glm::cross(crs, ba) + glm::cross(ca, crs) * glm::length2(ba)) /
    (2.f * glm::length2(crs));

  return Sphere(a + rvec, glm::length(rvec));
}

static void minBoundingSphereImpl(Sphere&          sp,
                                  const glm::vec3* begin,
                                  const glm::vec3* end,
                                  const glm::vec3* pin1 = nullptr,
                                  const glm::vec3* pin2 = nullptr,
                                  const glm::vec3* pin3 = nullptr)
{
  auto current = begin;
  if (pin1 && pin2 && pin3) {
    sp = triangleCircumsphere(*pin1, *pin2, *pin3);
    GALCAPTURE(sp);
  }
  else if (pin1 && pin2) {
    sp = Sphere::createFromDiameter(*pin1, *pin2);
    GALCAPTURE(sp);
  }
  else if (pin1) {
    sp = Sphere::createFromDiameter(*(current++), *pin1);
    GALCAPTURE(sp);
  }
  else {
    sp = Sphere::createFromDiameter(*current, *(current + 1));
    GALCAPTURE(sp);
    current += 2;
  }

  while (current != end) {
    if (!sp.contains(*current)) {
      if (pin1 && pin2 && pin3) {
        sp = Sphere::createCircumsphere(*pin1, *pin2, *pin3, *current);
      }
      else if (pin1 && pin2) {
        minBoundingSphereImpl(sp, begin, current, pin1, pin2, current);
      }
      else if (pin1) {
        minBoundingSphereImpl(sp, begin, current, pin1, current);
      }
      else {
        minBoundingSphereImpl(sp, begin, current, current);
      }
    }
    current++;
  }
}

Sphere Sphere::minBoundingSphere(const glm::vec3* pts, size_t npts)
{
  if (npts < 2) {
    throw "Cannot create minimum bounding sphere";
  }

  Sphere sp;
  minBoundingSphereImpl(sp, pts, pts + npts);
  return sp;
}

}  // namespace gal

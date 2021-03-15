#include <galcore/Box.h>

using namespace gal;
const Box3 Box3::empty = Box3(vec3_unset, -vec3_unset);
const Box2 Box2::empty = Box2(vec2_unset, -vec2_unset);

Box3::Box3()
    : min(vec3_unset)
    , max(-vec3_unset)
{}

Box3::Box3(const glm::vec3& min, const glm::vec3& max)
    : Box3()
{
  inflate(min);
  inflate(max);
}

Box3::Box3(const glm::vec3& pt)
    : min(pt)
    , max(pt)
{}

Box3::Box3(const glm::vec3* points, size_t nPoints)
    : Box3()
{
  for (size_t i = 0; i < nPoints; i++) {
    inflate(points[i]);
  }
}

glm::vec3 Box3::diagonal() const
{
  return max - min;
}

void Box3::inflate(const glm::vec3& pt)
{
  min = min_coords(pt, min);
  max = max_coords(pt, max);
}

void Box3::inflate(float d)
{
  glm::vec3 v(d, d, d);
  min -= v;
  max += v;
}

void Box3::deflate(float d)
{
  inflate(-d);
}

bool Box3::contains(const glm::vec3& pt) const
{
  return !(min.x > pt.x || max.x < pt.x || min.y > pt.y || max.y < pt.y || min.z > pt.z ||
           max.z < pt.z);
}

bool Box3::contains(const Box3& b) const
{
  return contains(b.min) && contains(b.max);
}

bool Box3::intersects(const Box3& b) const
{
  glm::vec3 m1 = max_coords(b.min, min);
  glm::vec3 m2 = min_coords(b.max, max);
  glm::vec3 d  = m2 - m1;
  return d.x > 0 && d.y > 0 && d.z > 0;
}

glm::vec3 Box3::center() const
{
  return (min + max) * 0.5f;
}

float Box3::volume() const
{
  glm::vec3 d = diagonal();
  return d.x * d.y * d.z;
}

Box3 Box3::init(const glm::vec3& m1, const glm::vec3& m2)
{
  Box3 b;
  b.min = m1;
  b.max = m2;
  return b;
}

glm::vec2 Box2::min_coords(const glm::vec2& a, const glm::vec2& b)
{
  return glm::vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}

glm::vec2 Box2::max_coords(const glm::vec2& a, const glm::vec2& b)
{
  return glm::vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

glm::vec3 Box3::min_coords(const glm::vec3& a, const glm::vec3& b)
{
  return glm::vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

glm::vec3 Box3::max_coords(const glm::vec3& a, const glm::vec3& b)
{
  return glm::vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
};

glm::vec3 Box3::eval(float u, float v, float w) const
{
  return glm::vec3((1.0f - u) * min.x + u * max.x,
                   (1.0f - v) * min.y + v * max.y,
                   (1.0f - w) * min.z + w * max.z);
};

Box2::Box2()
    : min(vec2_unset)
    , max(-vec2_unset)
{}

Box2::Box2(const glm::vec2& pt)
    : min(pt)
    , max(pt)
{}

Box2::Box2(const glm::vec2& a, const glm::vec2& b)
    : Box2()
{
  inflate(a);
  inflate(b);
}

Box2::Box2(const glm::vec2* points, size_t nPoints)
    : Box2()
{
  for (size_t i = 0; i < nPoints; i++) {
    inflate(points[i]);
  }
}

glm::vec2 Box2::diagonal() const
{
  return max - min;
}

void Box2::inflate(const glm::vec2& pt)
{
  min = min_coords(min, pt);
  max = max_coords(max, pt);
}

void Box2::inflate(float d)
{
  glm::vec2 v(d, d);
  min -= v;
  max += v;
}

void Box2::deflate(float d)
{
  inflate(-d);
}

bool Box2::contains(const glm::vec2& v) const
{
  return !(min.x > v.x || max.x < v.x || min.y > v.y || max.y < v.y);
}

bool Box2::contains(const Box2& b) const
{
  return contains(b.min) && contains(b.max);
}

bool Box2::intersects(const Box2& b) const
{
  glm::vec2 m1 = max_coords(min, b.min);
  glm::vec2 m2 = min_coords(max, b.max);
  glm::vec2 d  = m2 - m1;
  return d.x > 0 && d.y > 0;
}

glm::vec2 Box2::center() const
{
  return (min + max) * 0.5f;
}

Box2 Box2::init(const glm::vec2& m1, const glm::vec2& m2)
{
  Box2 b;
  b.min = m1;
  b.max = m2;
  return b;
}

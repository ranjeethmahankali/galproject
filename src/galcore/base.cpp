#include "galcore/base.h"
#include <cmath>

#define deletePtr(ptr, isArray) \
  if (isArray) {                \
    delete[] arr;               \
  }                             \
  else {                        \
    delete arr;                 \
  }

const box3 box3::empty = box3(vec3_unset, -vec3_unset);
const box2 box2::empty = box2(vec2_unset, -vec2_unset);

bool IndexPair::operator==(const IndexPair& pair) const
{
  return (p == pair.p && q == pair.q) || (p == pair.q && q == pair.p);
}

bool IndexPair::operator!=(const IndexPair& pair) const
{
  return (p != pair.q && p != pair.p) || (q != pair.p && q != pair.q);
}

IndexPair::IndexPair(size_t i, size_t j)
    : p(i)
    , q(j)
{}

IndexPair::IndexPair()
    : p(-1)
    , q(-1)
{}

void IndexPair::set(size_t i, size_t j)
{
  p = i;
  q = j;
}

size_t IndexPair::hash() const
{
  return p + q + p * q;
}

void IndexPair::unset(size_t i)
{
  if (p == i) {
    p = -1;
  }
  else if (q == i) {
    q = -1;
  }
}

bool IndexPair::add(size_t i)
{
  if (p == -1) {
    p = i;
    return true;
  }
  else if (q == -1) {
    q = i;
    return true;
  }
  return false;
}

bool IndexPair::contains(size_t i) const
{
  return (i != -1) && (i == p || i == q);
}

size_t IndexPairHash::operator()(const IndexPair& ip) const noexcept
{
  return ip.hash();
}

size_t CustomSizeTHash::operator()(const size_t& i) const noexcept
{
  return i;
}

box3::box3()
    : min(vec3_unset)
    , max(-vec3_unset)
{}

box3::box3(const glm::vec3& min, const glm::vec3& max)
    : box3()
{
  inflate(min);
  inflate(max);
}

box3::box3(const glm::vec3& pt)
    : min(pt)
    , max(pt)
{}

box3::box3(const glm::vec3* points, size_t nPoints)
    : box3()
{
  for (size_t i = 0; i < nPoints; i++) {
    inflate(points[i]);
  }
}

glm::vec3 box3::diagonal() const
{
  return max - min;
}

void box3::inflate(const glm::vec3& pt)
{
  min = min_coords(pt, min);
  max = max_coords(pt, max);
}

void box3::inflate(float d)
{
  glm::vec3 v(d, d, d);
  min -= v;
  max += v;
}

void box3::deflate(float d)
{
  inflate(-d);
}

bool box3::contains(const glm::vec3& pt) const
{
  return !(min.x > pt.x || max.x < pt.x || min.y > pt.y || max.y < pt.y || min.z > pt.z ||
           max.z < pt.z);
}

bool box3::contains(const box3& b) const
{
  return contains(b.min) && contains(b.max);
}

bool box3::intersects(const box3& b) const
{
  glm::vec3 m1 = max_coords(b.min, min);
  glm::vec3 m2 = min_coords(b.max, max);
  glm::vec3 d  = m2 - m1;
  return d.x > 0 && d.y > 0 && d.z > 0;
}

glm::vec3 box3::center() const
{
  return (min + max) * 0.5f;
}

float box3::volume() const
{
  glm::vec3 d = diagonal();
  return d.x * d.y * d.z;
}

box3 box3::init(const glm::vec3& m1, const glm::vec3& m2)
{
  box3 b;
  b.min = m1;
  b.max = m2;
  return b;
}

glm::vec2 box2::min_coords(const glm::vec2& a, const glm::vec2& b)
{
  return glm::vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}

glm::vec2 box2::max_coords(const glm::vec2& a, const glm::vec2& b)
{
  return glm::vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

glm::vec3 box3::min_coords(const glm::vec3& a, const glm::vec3& b)
{
  return glm::vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

glm::vec3 box3::max_coords(const glm::vec3& a, const glm::vec3& b)
{
  return glm::vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::min(a.z, b.z));
}

box2::box2()
    : min(vec2_unset)
    , max(-vec2_unset)
{}

box2::box2(const glm::vec2& pt)
    : min(pt)
    , max(pt)
{}

box2::box2(const glm::vec2& a, const glm::vec2& b)
    : box2()
{
  inflate(a);
  inflate(b);
}

box2::box2(const glm::vec2* points, size_t nPoints)
    : box2()
{
  for (size_t i = 0; i < nPoints; i++) {
    inflate(points[i]);
  }
}

glm::vec2 box2::diagonal() const
{
  return max - min;
}

void box2::inflate(const glm::vec2& pt)
{
  min = min_coords(min, pt);
  max = max_coords(max, pt);
}

void box2::inflate(float d)
{
  glm::vec2 v(d, d);
  min -= v;
  max += v;
}

void box2::deflate(float d)
{
  inflate(-d);
}

bool box2::contains(const glm::vec2& v) const
{
  return !(min.x > v.x || max.x < v.x || min.y > v.y || max.y < v.y);
}

bool box2::contains(const box2& b) const
{
  return contains(b.min) && contains(b.max);
}

bool box2::intersects(const box2& b) const
{
  glm::vec2 m1 = max_coords(min, b.min);
  glm::vec2 m2 = min_coords(max, b.max);
  glm::vec2 d  = m2 - m1;
  return d.x > 0 && d.y > 0;
}

glm::vec2 box2::center() const
{
  return (min + max) * 0.5f;
}

box2 box2::init(const glm::vec2& m1, const glm::vec2& m2)
{
  box2 b;
  b.min = m1;
  b.max = m2;
  return b;
}

bool utils::barycentricWithinBounds(float const (&coords)[3])
{
  return 0 <= coords[0] && coords[0] <= 1 && 0 <= coords[1] && coords[1] <= 1 &&
         0 <= coords[2] && coords[2] <= 1;
}

glm::vec3 utils::barycentricEvaluate(float const (&coords)[3], glm::vec3 const (&pts)[3])
{
  return pts[0] * coords[0] + pts[1] * coords[1] + pts[2] * coords[2];
}

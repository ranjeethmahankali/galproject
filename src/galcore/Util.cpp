#include <galcore/Util.h>
#include <cmath>

#define deletePtr(ptr, isArray) \
  if (isArray) {                \
    delete[] arr;               \
  }                             \
  else {                        \
    delete arr;                 \
  }

bool gal::IndexPair::operator==(const gal::IndexPair& pair) const
{
  return (p == pair.p && q == pair.q) || (p == pair.q && q == pair.p);
}

bool gal::IndexPair::operator!=(const gal::IndexPair& pair) const
{
  return (p != pair.q && p != pair.p) || (q != pair.p && q != pair.q);
}

gal::IndexPair::IndexPair(size_t i, size_t j)
    : p(i)
    , q(j)
{}

gal::IndexPair::IndexPair()
    : p(-1)
    , q(-1)
{}

void gal::IndexPair::set(size_t i, size_t j)
{
  p = i;
  q = j;
}

size_t gal::IndexPair::hash() const
{
  return p + q + p * q;
}

void gal::IndexPair::unset(size_t i)
{
  if (p == i) {
    p = -1;
  }
  else if (q == i) {
    q = -1;
  }
}

bool gal::IndexPair::add(size_t i)
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

bool gal::IndexPair::contains(size_t i) const
{
  return (i != -1) && (i == p || i == q);
}

bool gal::utils::barycentricWithinBounds(float const (&coords)[3])
{
  return 0 <= coords[0] && coords[0] <= 1 && 0 <= coords[1] && coords[1] <= 1 &&
         0 <= coords[2] && coords[2] <= 1;
}

glm::vec3 gal::utils::barycentricEvaluate(float const (&coords)[3],
                                          glm::vec3 const (&pts)[3])
{
  return pts[0] * coords[0] + pts[1] * coords[1] + pts[2] * coords[2];
}

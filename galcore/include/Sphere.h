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

  bool contains(const glm::vec3& pt, float tolerance = 0.f) const;

  static Sphere createCircumsphere(const glm::vec3& a,
                                   const glm::vec3& b,
                                   const glm::vec3& c,
                                   const glm::vec3& d);
  static Sphere createFromDiameter(const glm::vec3& a, const glm::vec3& b);

  /**
   * @brief Computes the minimum bounding circle for the given points.
   *
   * @param pts Pointer to the range of points.
   * @param nPts The number of points.
   * @return Sphere Minimum bounding sphere.
   */
  static Sphere minBoundingSphere(const glm::vec3* pts, size_t nPts);

  /**
   * @brief Computes the minimum bounding circle for the given points.
   *
   * @tparam VectorT The container type of glm::vec2. Must use contiguous storage and have
   * member functions data() and size().
   * @param points container of points.
   * @return Sphere result.
   */
  template<typename VectorT>
  inline static Sphere minBoundingSphere(const VectorT& points)
  {
    return minBoundingSphere(points.data(), points.size());
  }
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

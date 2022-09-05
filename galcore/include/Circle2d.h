#pragma once

#include <glm/glm.hpp>

#include <Box.h>
#include <Serialization.h>

namespace gal {

class Circle2d
{
public:
  Circle2d() = default;
  Circle2d(const glm::vec2& center, float radius);

  /**
   * @brief
   * @return const glm::vec2&
   */
  const glm::vec2& center() const;
  void             center(const glm::vec2& newCenter);
  float            radius() const;
  void             radius(float newRadius);
  bool             contains(const glm::vec2&, float tolerance = 0.f) const;
  float            area() const;
  float            perimeter() const;

  Box2 bounds() const;

  static Circle2d createCircumcircle(const glm::vec2& a,
                                     const glm::vec2& b,
                                     const glm::vec2& c);
  static Circle2d createFromDiameter(const glm::vec2& a, const glm::vec2& b);

  /**
   * @brief Computes the minimum bounding circle for the given points.
   *
   * @param pts Pointer to the range of points.
   * @param npts The number of points.
   * @return Circle2d The result.
   */
  static Circle2d minBoundingCircle(const glm::vec2* pts, size_t npts);

  /**
   * @brief Computes the minimum bounding circle for the given points.
   *
   * @tparam VectorT Container type of glm::vec2 instances. Must use contiguous storage,
   * and have member functions data() and size().
   * @param points The container of points.
   * @return Circle2d result.
   */
  template<typename VectorT>
  inline static Circle2d minBoundingCircle(const VectorT& points)
  {
    return minBoundingCircle(points.data(), points.size());
  }

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

  static Bytes serialize(const Circle2d& circ)
  {
    Bytes bytes;
    bytes << circ.center() << circ.radius();
    return bytes;
  }
};

}  // namespace gal

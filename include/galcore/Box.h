#pragma once
#include <galcore/Serialization.h>
#include <galcore/Util.h>

namespace gal {

struct Box2;

struct Box3
{
  static const Box3 empty;
  glm::vec3         min, max;

  Box3();
  Box3(const glm::vec3& min, const glm::vec3& max);
  Box3(const glm::vec3& pt);
  Box3(const glm::vec3* points, size_t nPoints);
  Box3(const Box2& b2);

  glm::vec3 diagonal() const;
  void      inflate(const glm::vec3&);
  void      inflate(float);
  void      deflate(float);
  bool      contains(const glm::vec3&) const;
  bool      contains(const Box3&) const;
  bool      intersects(const Box3&) const;
  glm::vec3 center() const;
  float     volume() const;
  bool      valid() const;

  glm::vec3 eval(float u, float v, float w) const;

  template<typename DstIter>
  void randomPoints(size_t n, DstIter dst) const
  {
    std::vector<float> x(n), y(n), z(n);
    utils::random(min.x, max.x, n, x.data());
    utils::random(min.y, max.y, n, y.data());
    utils::random(min.z, max.z, n, z.data());
    for (size_t i = 0; i < n; i++) {
      *(dst++) = glm::vec3(x[i], y[i], z[i]);
    };
  };

  static Box3      init(const glm::vec3&, const glm::vec3&);
  static glm::vec3 max_coords(const glm::vec3& a, const glm::vec3& b);
  static glm::vec3 min_coords(const glm::vec3& a, const glm::vec3& b);
};

template<>
struct Serial<Box3> : public std::true_type
{
  static Box3 deserialize(Bytes& bytes)
  {
    Box3 box;
    bytes >> box.min >> box.max;
    return box;
  }
  
  static Bytes serialize(const Box3& box)
  {
    Bytes bytes;
    bytes << box.min << box.max;
    return bytes;
  }
};

struct Box2
{
  static const Box2 empty;
  glm::vec2         min, max;

  Box2();
  Box2(const glm::vec2& pt);
  Box2(const glm::vec2&, const glm::vec2&);
  Box2(const glm::vec2* points, size_t nPoints);

  glm::vec2 diagonal() const;
  void      inflate(const glm::vec2&);
  void      inflate(float);
  void      deflate(float);
  bool      contains(const glm::vec2&) const;
  bool      contains(const Box2&) const;
  bool      intersects(const Box2&) const;
  glm::vec2 center() const;
  float     area() const;
  bool      valid() const;

  template<typename DstIter>
  void randomPoints(size_t n, DstIter dst) const
  {
    std::vector<float> x(n), y(n);
    utils::random(min.x, max.x, n, x.data());
    utils::random(min.y, max.y, n, y.data());
    for (size_t i = 0; i < n; i++) {
      *(dst++) = glm::vec2(x[i], y[i]);
    };
  };

  static Box2      init(const glm::vec2&, const glm::vec2&);
  static glm::vec2 max_coords(const glm::vec2& a, const glm::vec2& b);
  static glm::vec2 min_coords(const glm::vec2& a, const glm::vec2& b);
};

}  // namespace gal

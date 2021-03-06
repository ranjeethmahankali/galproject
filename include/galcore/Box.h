#pragma once
#include <galcore/Util.h>

namespace gal {

struct box3
{
  static const box3 empty;
  glm::vec3         min, max;

  box3();
  box3(const glm::vec3& min, const glm::vec3& max);
  box3(const glm::vec3& pt);
  box3(const glm::vec3* points, size_t nPoints);

  glm::vec3 diagonal() const;
  void      inflate(const glm::vec3&);
  void      inflate(float);
  void      deflate(float);
  bool      contains(const glm::vec3&) const;
  bool      contains(const box3&) const;
  bool      intersects(const box3&) const;
  glm::vec3 center() const;
  float     volume() const;

  static box3      init(const glm::vec3&, const glm::vec3&);
  static glm::vec3 max_coords(const glm::vec3& a, const glm::vec3& b);
  static glm::vec3 min_coords(const glm::vec3& a, const glm::vec3& b);
};

struct box2
{
  static const box2 empty;
  glm::vec2         min, max;

  box2();
  box2(const glm::vec2& pt);
  box2(const glm::vec2&, const glm::vec2&);
  box2(const glm::vec2* points, size_t nPoints);

  glm::vec2 diagonal() const;
  void      inflate(const glm::vec2&);
  void      inflate(float);
  void      deflate(float);
  bool      contains(const glm::vec2&) const;
  bool      contains(const box2&) const;
  bool      intersects(const box2&) const;
  glm::vec2 center() const;

  static box2      init(const glm::vec2&, const glm::vec2&);
  static glm::vec2 max_coords(const glm::vec2& a, const glm::vec2& b);
  static glm::vec2 min_coords(const glm::vec2& a, const glm::vec2& b);
};

}  // namespace gal

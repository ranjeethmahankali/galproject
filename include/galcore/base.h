#pragma once
#include <float.h>
#include <stdint.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>
#include <vector>

static constexpr glm::vec3 vec3_zero  = {0.0f, 0.0f, 0.0f};
static constexpr glm::vec3 vec3_unset = {FLT_MAX, FLT_MAX, FLT_MAX};
static constexpr glm::vec2 vec2_zero  = {0.0f, 0.0f};
static constexpr glm::vec2 vec2_unset = {FLT_MAX, FLT_MAX};

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

struct IndexPair
{
  size_t p, q;

  bool operator==(const IndexPair&) const;
  bool operator!=(const IndexPair&) const;

  IndexPair(size_t i, size_t j);
  IndexPair();

  void   set(size_t, size_t);
  size_t hash() const;
  void   unset(size_t);
  bool   add(size_t);
  bool   contains(size_t) const;
};

struct IndexPairHash
{
  size_t operator()(const IndexPair&) const noexcept;
};

struct CustomSizeTHash
{
  size_t operator()(const size_t&) const noexcept;
};

namespace utils {

template<typename vtype>
void barycentricCoords(vtype const (&tri)[3], const vtype& pt, float (&coords)[3])
{
  vtype       v0 = tri[1] - tri[0], v1 = tri[2] - tri[0], v2 = pt - tri[0];
  const float d00   = glm::dot(v0, v0);
  const float d01   = glm::dot(v0, v1);
  const float d11   = glm::dot(v1, v1);
  const float d20   = glm::dot(v2, v0);
  const float d21   = glm::dot(v2, v1);
  const float denom = d00 * d11 - d01 * d01;
  coords[1]         = denom == 0 ? DBL_MAX : (d11 * d20 - d01 * d21) / denom;
  coords[2]         = denom == 0 ? DBL_MAX : (d00 * d21 - d01 * d20) / denom;
  coords[0]         = denom == 0 ? DBL_MAX : 1.0 - coords[1] - coords[2];
};

bool barycentricWithinBounds(float const (&coords)[3]);

glm::vec3 barycentricEvaluate(float const (&coords)[3], glm::vec3 const (&pts)[3]);

template<typename T, typename TIter>
T average(TIter begin, TIter end);

template<typename T, typename WT, typename TIter, typename WIter>
T weightedAverage(TIter begin, TIter end, WIter wbegin);

constexpr bool isValid(const glm::vec3& v)
{
  return v != vec3_unset;
}

constexpr bool isValid(const glm::vec2& v)
{
  return v != vec2_unset;
}

}  // namespace utils

#pragma once
#include <float.h>
#include <galcore/Serialization.h>
#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

static constexpr glm::vec3 vec3_zero  = {0.0f, 0.0f, 0.0f};
static constexpr glm::vec3 vec3_xunit = {1.0f, 0.0f, 0.0f};
static constexpr glm::vec3 vec3_yunit = {0.0f, 1.0f, 0.0f};
static constexpr glm::vec3 vec3_zunit = {0.0f, 0.0f, 1.0f};
static constexpr glm::vec3 vec3_unset = {FLT_MAX, FLT_MAX, FLT_MAX};

static constexpr glm::vec2 vec2_zero  = {0.0f, 0.0f};
static constexpr glm::vec2 vec2_xunit = {1.0f, 0.0f};
static constexpr glm::vec2 vec2_yunit = {0.0f, 1.0f};
static constexpr glm::vec2 vec2_unset = {FLT_MAX, FLT_MAX};

namespace std {
std::ostream& operator<<(std::ostream& ostr, const glm::vec3& v);
std::ostream& operator<<(std::ostream& ostr, const glm::vec2& v);
}  // namespace std

namespace fs = std::filesystem;

namespace gal {

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

template<typename T>
struct Hash
{
  static const std::hash<T> mHasher;

  size_t operator()(const T& v) const noexcept { return mHasher(v); }
};

template<>
struct Hash<IndexPair>
{
  size_t operator()(const IndexPair& ip) const noexcept { return ip.hash(); }
};
using IndexPairHash = Hash<IndexPair>;

template<>
struct Hash<size_t>
{
  size_t operator()(const size_t& v) const noexcept { return v; };
};
using CustomSizeTHash = Hash<size_t>;

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

template<typename TIter>
glm::vec3 average(TIter begin, TIter end)
{
  glm::vec3 sum = vec3_zero;
  size_t    n   = 0;
  while (begin != end) {
    const glm::vec3& v = *(begin++);
    sum += v;
    n++;
  }
  return sum / float(n);
}

template<typename TIter, typename WIter>
glm::vec3 weightedAverage(TIter begin, TIter end, WIter wbegin)
{
  glm::vec3 sum   = vec3_zero;
  float     denom = 0.0f;
  while (begin != end) {
    float            w = *(wbegin++);
    const glm::vec3& v = *(begin++);
    sum += v * w;
    denom += w;
  }
  return sum / denom;
}

constexpr bool isValid(const glm::vec3& v)
{
  return v != vec3_unset;
}

constexpr bool isValid(const glm::vec2& v)
{
  return v != vec2_unset;
}

template<typename T>
void copy_coords(const glm::tvec3<T>& v, T*& dst)
{
  *(dst++) = v.x;
  *(dst++) = v.y;
  *(dst++) = v.z;
};

fs::path absPath(const fs::path& relPath);

template<typename T, typename DstIter>
void random(T min, T max, size_t count, DstIter dst)
{
  for (size_t i = 0; i < count; i++) {
    if constexpr (std::is_integral_v<T>) {
      T span   = max - min;
      *(dst++) = min + (static_cast<T>(rand()) % span);
    }
    else if constexpr (std::is_floating_point_v<T>) {
      static constexpr T TMax = static_cast<T>(RAND_MAX);
      T                  span = max - min;
      T                  val  = static_cast<T>(rand());
      *(dst++)                = min + span * (val / TMax);
    }
    else {
      std::cerr << "Cannot generate random numbers of given type!\n";
    }
  }
};

}  // namespace utils
}  // namespace gal

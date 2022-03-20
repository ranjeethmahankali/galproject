#pragma once

#include <float.h>
#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/range/iterator_range_core.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <galcore/Traits.h>

static constexpr glm::vec3 vec3_zero  = {0.0f, 0.0f, 0.0f};
static constexpr glm::vec3 vec3_xunit = {1.0f, 0.0f, 0.0f};
static constexpr glm::vec3 vec3_yunit = {0.0f, 1.0f, 0.0f};
static constexpr glm::vec3 vec3_zunit = {0.0f, 0.0f, 1.0f};
static constexpr glm::vec3 vec3_unset = {FLT_MAX, FLT_MAX, FLT_MAX};

static constexpr glm::vec2 vec2_zero  = {0.0f, 0.0f};
static constexpr glm::vec2 vec2_xunit = {1.0f, 0.0f};
static constexpr glm::vec2 vec2_yunit = {0.0f, 1.0f};
static constexpr glm::vec2 vec2_unset = {FLT_MAX, FLT_MAX};

namespace fs = std::filesystem;

namespace std {
std::ostream& operator<<(std::ostream& ostr, const glm::vec3& v);
std::ostream& operator<<(std::ostream& ostr, const glm::vec2& v);

/**
 * @brief Writes the object to the stream safely, If the stream operator exists for the
 * type. Or else a placeholder label is printed.
 *
 * @tparam T
 * @param ostr Destination stream.
 * @param obj Object to be written.
 */
template<typename T>
inline std::ostream& safeWrite(std::ostream& ostr, const T& obj)
{
  if constexpr (gal::IsPrintable<T>::value) {
    ostr << obj;
  }
  else {
    ostr << "Unprintable object";
  }
  return ostr;
}

template<typename T>
std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& vec)
{
  static constexpr char tab = '\t';
  ostr << "[\n";
  for (const T& v : vec) {
    ostr << tab;
    safeWrite(ostr, v);
    ostr << std::endl;
  }
  ostr << "]";
  return ostr;
};

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& pair)
{
  ostr << "(";
  safeWrite(ostr, pair.first);
  safeWrite(ostr, pair.second);
  ostr << ")";
  return ostr;
}

}  // namespace std

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

spdlog::logger& logger();

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
  static constexpr char sErrorMsg[] = "Cannot generate random values of given type!\n";
  for (size_t i = 0; i < count; i++) {
    if constexpr (std::is_integral_v<T>) {
      *(dst++) = min + (static_cast<T>(std::rand()) % (max - min));
    }
    else if constexpr (std::is_floating_point_v<T>) {
      static constexpr T TMax = static_cast<T>(RAND_MAX);
      *(dst++)                = min + (max - min) * (static_cast<T>(std::rand()) / TMax);
    }
    else if constexpr (GlmVecTraits<T>::IsGlmVec) {
      using TVal = typename GlmVecTraits<T>::ValueType;
      T v;
      for (int i = 0; i < GlmVecTraits<T>::Size; i++) {
        if constexpr (std::is_integral_v<TVal>) {
          v[i] = min[i] + (static_cast<TVal>(std::rand()) % (max[i] - min[i]));
        }
        else if constexpr (std::is_floating_point_v<TVal>) {
          static constexpr TVal TValMax = static_cast<TVal>(RAND_MAX);
          v[i] = min[i] + (max[i] - min[i]) * (static_cast<TVal>(std::rand()) / TValMax);
        }
        else {
          std::cerr << sErrorMsg;
          return;
        }
      }
      *(dst++) = v;
    }
    else {
      std::cerr << sErrorMsg;
    }
  }
};

/**
 * @brief Scans the bits of the integer and returns the position of the first set bit. By
 * default the bits are scanned from the least significant to the most significant.
 *
 * @tparam T The type of the integer. Must be unsigned.
 * @tparam Reverse If this is true, the bits are scanned from the most significant to the
 * least.
 * @param i The integer.
 * @return int The position of the first set bit. -1 if i is zero.
 */
template<typename T, bool Reverse = false>
inline int bitscan(T i)
{
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                "Unsupported type for bitscan");
  if (i == 0) {
    return -1;
  }
  if constexpr (std::is_same_v<T, uint32_t>) {
    if constexpr (Reverse) {
      return 31 - __builtin_clz(i);
    }
    else {
      return __builtin_ctz(i);
    }
  }
  else if constexpr (std::is_same_v<T, uint64_t>) {
    if constexpr (Reverse) {
      return 63 - __builtin_clzl(i);
    }
    else {
      return __builtin_ctzl(i);
    }
  }
  else {
    return bitscan<uint64_t, Reverse>(uint64_t(i));
  }
}

template<typename T>
inline int bitscanForward(T i)
{
  return bitscan<T, false>(i);
}

template<typename T>
inline int bitscanReverse(T i)
{
  return bitscan<T, true>(i);
}

/**
 * @brief Gets the number of combinations of a given length for a set of given size.
 *
 * @param n The size of the set.
 * @param k The size of a single combination.
 * @return size_t The total number of combinations.
 */
size_t numCombinations(size_t n, size_t k);

/**
 * @brief Generates all possible combinations of a given length
 * from the given range of elements.
 *
 * @tparam TIter Iterator type.
 * @tparam TOutIter Output iterator type.
 * @tparam TCallable Callable type.
 * @param k The number of elements in a single combination.
 * @param begin The beginning of the range of elements.
 * @param end The end of the range of elements.
 * @param dst The k elements corresponding to a combination are written to this.
 * @param notify Every time dst is populated with k elements corresponding to a single
 * combination, this callback is called, which the caller can use to consume the
 * combination.
 */
template<typename TIter, typename TOutIter, typename TCallable>
void combinations(size_t k, TIter begin, TIter end, TOutIter dst, const TCallable& notify)
{
  if (begin == end || k == 0) {
    return;
  }

  while (begin != end) {
    *dst = *(begin++);
    if (k > 1) {
      combinations(k - 1, begin, end, dst + 1, notify);
    }
    else if (k == 1) {
      notify();
    }
  }
}

/**
 * @brief Check the sign of the value.
 *
 * @tparam T
 * @param val
 * @return int 1 if positive, 0 if 0, -1 if negative.
 */
template<typename T>
int sign(T val)
{
  return (T(0) < val) - (val < T(0));
}

template<typename T, typename... Ts>
constexpr std::array<T, sizeof...(Ts)> makeArray(const Ts&... vals)
{
  static_assert((std::is_constructible_v<T, Ts> && ...));
  return {{T(vals)...}};
};

template<typename T, typename ContainerT>
std::vector<T> makeVector(const ContainerT& c)
{
  return std::vector<T>(c.begin(), c.end());
}

template<typename T>
using IterSpan = boost::iterator_range<T>;

/**
 * @brief Wrapper that treats the wrapped instance as cached data. So you can expire the
 * cache, and it will be automatically updated when you try to access using the given
 * update-functor. The usage is similar to std::optional.
 *
 * @tparam T The wrapped datatype.
 */
template<typename T>
struct Cached
{
  using UpdateFuncT = std::function<void(T&)>;

private:
  T           mValue;
  UpdateFuncT mUpdateFn;
  bool        mIsExpired = true;

public:
  template<bool B                                = std::is_default_constructible_v<T>,
           typename std::enable_if<B, int>::type = 0>
  Cached()
      : mValue()
  {}

  template<typename... Args>
  explicit Cached(UpdateFuncT updatefn, const Args&... args)
      : mValue(args...)
      , mUpdateFn(std::move(updatefn))
  {}

  void expire() { mIsExpired = true; }
  void ensure()
  {
    if (mIsExpired) {
      mUpdateFn(mValue);
      mIsExpired = false;
    }
  }

  bool isExpired() const { return mIsExpired; }
  T&   value()
  {
    ensure();
    return mValue;
  }
  const T& value() const
  {
    ensure();
    return mValue;
  }
  T&       operator*() { return value(); }
  const T& operator*() const { return value(); }
  T*       operator->() { return &value(); }
  const T* operator->() const { return &value(); }
};

}  // namespace utils
}  // namespace gal

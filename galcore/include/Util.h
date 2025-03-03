#pragma once

#include <float.h>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

#include <Traits.h>

#ifdef _MSC_VER
#include <Windows.h>
#endif

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

template<int N, typename T>
std::ostream& operator<<(std::ostream& ostr, const glm::vec<N, T>& v)
{
  ostr << "(" << v[0];
  for (int i = 1; i < N; ++i) {
    ostr << ", " << v[i];
  }
  ostr << ")";
  return ostr;
}

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

namespace utils {

spdlog::logger& logger();

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
  static constexpr std::string_view sErrorMsg =
    "Cannot generate random values of given type!\n";
  for (size_t i = 0; i < count; i++) {
    if constexpr (std::is_integral_v<T>) {
      *(dst++) = min + (static_cast<T>(std::rand()) % (max - min));
    }
    else if constexpr (std::is_floating_point_v<T>) {
      static constexpr T TMax = static_cast<T>(RAND_MAX);
      *(dst++)                = min + (max - min) * (static_cast<T>(std::rand()) / TMax);
    }
    else if constexpr (GlmVecTraits<T>::IsGlmVec) {
      T v;
      for (int j = 0; j < GlmVecTraits<T>::Size; j++) {
        random(min[j], max[j], 1, &v[j]);
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
int bitscanForward(uint32_t i);
int bitscanForward(uint64_t i);

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
    else {
      assert(k == 1);
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

/**
 * @brief Wrapper that treats the wrapped instance as cached data. You
 * can keep track of whether the cache is expired and needs to be
 * recomputed. The usage is similar to std::optional.
 *
 * @tparam T The wrapped datatype.
 */
template<typename T>
struct Cached
{
private:
  T          mValue;
  bool       mIsExpired = true;
  std::mutex mMutex;

public:
  template<typename... Args>
  explicit Cached(const Args&... args)
      : mValue(args...)
  {}

  Cached(const Cached& other)
      : mValue(other.mValue)
      , mIsExpired(other.mIsExpired)
      , mMutex()  // Don't copy the mutex
  {}

  ~Cached() = default;

  Cached& operator=(const Cached& other)
  {
    mValue     = other.mValue;
    mIsExpired = other.mIsExpired;
    return *this;
  }

  Cached(Cached&& other)
      : mValue(std::move(other.mValue))
      , mIsExpired(other.mIsExpired)
      , mMutex()
  {}

  Cached& operator=(Cached&& other)
  {
    mValue     = std::move(other.mValue);
    mIsExpired = other.mIsExpired;
    return *this;
  }

  void expire() { mIsExpired = true; }
  bool isExpired() const { return mIsExpired; }
  void unexpire() { mIsExpired = false; }

  T&       value() { return mValue; }
  const T& value() const { return mValue; }
  T&       operator*() { return value(); }
  const T& operator*() const { return value(); }
  T*       operator->() { return &value(); }
  const T* operator->() const { return &value(); }
           operator bool() const { return !isExpired(); }

  std::mutex& mutex() { return mMutex; }
};

}  // namespace utils
}  // namespace gal

#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <PointCloud.h>
#include <Serialization.h>

namespace gal {

struct Glyph
{
  uint32_t mIndex = UINT32_MAX;
};

template<>
struct IsValueType<Glyph> : public std::true_type
{
};

template<typename T>
using PositionalData = std::pair<glm::vec3, T>;

template<typename T>
class Annotations : public std::vector<PositionalData<T>>
{
public:
  Annotations() = default;
  explicit Annotations(size_t size)
      : std::vector<PositionalData<T>>(size)
  {}
  explicit Annotations(std::vector<PositionalData<T>> tags)
      : std::vector<PositionalData<T>>(std::move(tags))
  {}
};

using TextAnnotations  = Annotations<std::string>;
using GlyphAnnotations = Annotations<Glyph>;

TextAnnotations createIndexedPoints(const std::vector<glm::vec3>& points);
TextAnnotations createIndexedPointCloud(const PointCloud<3>& cloud);

template<typename T>
struct Serial<Annotations<T>> : public std::true_type
{
  static Annotations<T> deserialize(Bytes& bytes)
  {
    std::vector<PositionalData<T>> tags;
    bytes >> tags;
    return Annotations(std::move(tags));
  }

  static Bytes serialize(const Annotations<T>& tags)
  {
    Bytes bytes;
    bytes << (const std::vector<PositionalData<T>>&)tags;
    return bytes;
  }
};

}  // namespace gal

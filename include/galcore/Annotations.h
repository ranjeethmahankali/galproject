#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <galcore/PointCloud.h>
#include <galcore/Serialization.h>

namespace gal {

using PositionalTextType = std::pair<glm::vec3, std::string>;

class Annotations : public std::vector<PositionalTextType>
{
public:
  Annotations() = default;
  Annotations(size_t size);
  Annotations(std::vector<PositionalTextType> tags);

  static Annotations createIndexedPoints(const std::vector<glm::vec3>& points);
  static Annotations createIndexedPointCloud(const PointCloud& cloud);
};

template<>
struct Serial<Annotations> : public std::true_type
{
  static Annotations deserialize(Bytes& bytes)
  {
    std::vector<PositionalTextType> tags;
    bytes >> tags;
    return Annotations(std::move(tags));
  }

  static Bytes serialize(const Annotations& tags)
  {
    Bytes bytes;
    bytes << (const std::vector<PositionalTextType>&)tags;
    return bytes;
  }
};

}  // namespace gal
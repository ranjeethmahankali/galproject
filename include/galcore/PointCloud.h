#pragma once
#include <galcore/Serialization.h>
#include <glm/glm.hpp>
#include <galcore/Box.h>
#include <vector>

namespace gal {
class PointCloud : public std::vector<glm::vec3>
{
public:
  PointCloud() = default;
  PointCloud(const std::vector<glm::vec3>&);
  PointCloud(const std::vector<glm::vec2>& pts2d);

  Box3 bounds() const;
};

template<>
struct Serial<PointCloud> : public std::true_type
{
  static PointCloud deserialize(Bytes& bytes)
  {
    PointCloud cloud;
    uint64_t   npts = 0;
    bytes >> npts;
    cloud.resize(npts);
    for (auto& pt : cloud) {
      bytes >> pt;
    }
    return cloud;
  }
  static Bytes serialize(const PointCloud& cloud)
  {
    Bytes dst;
    dst << uint64_t(cloud.size());
    for (const auto& pt : cloud) {
      dst << pt;
    }
    return dst;
  }
};

}  // namespace gal
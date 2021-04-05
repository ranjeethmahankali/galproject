#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace gal {
class PointCloud : public std::vector<glm::vec3>
{
public:
  PointCloud() = default;
  PointCloud(const std::vector<glm::vec2>& pts2d);
};
}  // namespace gal
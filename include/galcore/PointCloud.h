#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace gal {
class PointCloud : public std::vector<glm::vec3>
{
};
}  // namespace gal
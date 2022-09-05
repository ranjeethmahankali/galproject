#include <galcore/PointCloud.h>
#include <span>

namespace gal {

PointCloud::PointCloud(const std::vector<glm::vec3>& pts)
    : std::vector<glm::vec3>(pts) {};

PointCloud::PointCloud(const std::vector<glm::vec2>& pts2d)
{
  reserve(pts2d.size());
  for (const auto& p : pts2d) {
    emplace_back(p.x, p.y, 0.f);
  }
};

Box3 PointCloud::bounds() const
{
  return Box3(std::span<glm::vec3>((std::vector<glm::vec3>&)(*this)));
}

};  // namespace gal

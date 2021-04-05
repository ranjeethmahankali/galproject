#include <galcore/PointCloud.h>

namespace gal {

PointCloud::PointCloud(const std::vector<glm::vec2>& pts2d)
{
    reserve(pts2d.size());
    for (const auto& p : pts2d) {
        emplace_back(p.x, p.y, 0.f);
    }
};

};  // namespace gal
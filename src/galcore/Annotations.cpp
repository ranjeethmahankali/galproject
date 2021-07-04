#include <galcore/Annotations.h>

namespace gal {

Annotations::Annotations(size_t size)
    : std::vector<PositionalTextType>(size)
{}

Annotations::Annotations(std::vector<PositionalTextType> tags)
    : std::vector<PositionalTextType>(std::move(tags))
{}

Annotations Annotations::createIndexedPoints(const std::vector<glm::vec3>& points)
{
  Annotations tags(points.size());
  size_t      i = 0;
  std::transform(points.begin(), points.end(), tags.begin(), [&i](const glm::vec3& pos) {
    return std::make_pair(pos, std::to_string(i++));
  });
  return tags;
}

Annotations Annotations::createIndexedPointCloud(const gal::PointCloud& cloud)
{
  return createIndexedPoints((const std::vector<glm::vec3>&)cloud);
}

}  // namespace gal

#include <Annotations.h>

namespace gal {

TextAnnotations createIndexedPoints(const std::vector<glm::vec3>& points)
{
  Annotations<std::string> tags(points.size());
  size_t                   i = 0;
  std::transform(points.begin(), points.end(), tags.begin(), [&i](const glm::vec3& pos) {
    return std::make_pair(pos, std::to_string(i++));
  });
  return tags;
}

TextAnnotations createIndexedPointCloud(const gal::PointCloud<3>& cloud)
{
  return createIndexedPoints((const std::vector<glm::vec3>&)cloud);
}

}  // namespace gal

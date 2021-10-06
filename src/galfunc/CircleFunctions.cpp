#include <galfunc/CircleFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(boundingCircle,
              ((gal::PointCloud, points)),
              ((gal::Circle2d, circle), (glm::vec2, center), (float, radius)))
{
  std::vector<glm::vec2> pts2d;
  pts2d.reserve(points.size());
  std::transform(
    points.begin(), points.end(), std::back_inserter(pts2d), [](const glm::vec3& p) {
      return glm::vec2(p);
    });
  circle = gal::Circle2d::minBoundingCircle(pts2d);
  center = circle.center();
  radius = circle.radius();
}

GAL_FUNC_DEFN(circle2d, ((glm::vec2, center), (float, radius)), ((gal::Circle2d, circle)))
{
  circle.center(center);
  circle.radius(radius);
}

GAL_FUNC_DEFN(circle2dFromDiameter,
              ((glm::vec2, pt1), (glm::vec2, pt2)),
              ((gal::Circle2d, circle), (glm::vec2, center), (float, radius)))
{
  circle = gal::Circle2d::createFromDiameter(pt1, pt2);
  center = circle.center();
  radius = circle.radius();
}

GAL_FUNC_DEFN(circumCircle2d,
              ((glm::vec2, pt1), (glm::vec2, pt2), (glm::vec2, pt3)),
              ((gal::Circle2d, circle), (glm::vec2, center), (float, radius)))
{
  circle = gal::Circle2d::createCircumcircle(pt1, pt2, pt3);
  center = circle.center();
  radius = circle.radius();
}

}  // namespace func
}  // namespace gal

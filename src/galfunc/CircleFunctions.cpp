#include <galfunc/CircleFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(((gal::Circle2d, circle, "Bounding circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              boundingCircle,
              true,
              1,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (gal::PointCloud, points, "Points"))
{
  std::vector<glm::vec2> pts2d;
  pts2d.reserve(points->size());
  std::transform(
    points->begin(), points->end(), std::back_inserter(pts2d), [](const glm::vec3& p) {
      return glm::vec2(p);
    });
  auto      circ = gal::Circle2d::minBoundingCircle(pts2d);
  glm::vec2 cent = circ.center();
  float     rad  = circ.radius();
  return std::make_tuple(std::make_shared<gal::Circle2d>(std::move(circ)),
                         std::make_shared<glm::vec2>(cent),
                         std::make_shared<float>(rad));
}

GAL_FUNC_DEFN(((gal::Circle2d, circle, "Circle")),
              circle2d,
              true,
              2,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (glm::vec2, center, "Center"),
              (float, radius, "Radius"))
{
  return std::make_tuple(std::make_shared<gal::Circle2d>(*center, *radius));
}

GAL_FUNC_DEFN(((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              circle2dFromDiameter,
              true,
              2,
              "Creates a 2d circle with the given points as the ends of the diameter",
              (glm::vec2, pt1, "First point"),
              (glm::vec2, pt2, "Second point"))
{
  auto      circ   = gal::Circle2d::createFromDiameter(*pt1, *pt2);
  glm::vec2 center = circ.center();
  float     rad    = circ.radius();
  return std::make_tuple(std::make_shared<gal::Circle2d>(std::move(circ)),
                         std::make_shared<glm::vec2>(center),
                         std::make_shared<float>(rad));
}

GAL_FUNC_DEFN(((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              circumCircle2d,
              true,
              3,
              "Creates a 2d circle with the given points as the ends of the diameter",
              (glm::vec2, pt1, "First point"),
              (glm::vec2, pt2, "Second point"),
              (glm::vec2, pt3, "Third point"))
{
  auto      circ   = gal::Circle2d::createCircumcircle(*pt1, *pt2, *pt3);
  glm::vec2 center = circ.center();
  float     radius = circ.radius();
  return std::make_tuple(std::make_shared<gal::Circle2d>(std::move(circ)),
                         std::make_shared<glm::vec2>(center),   
                         std::make_shared<float>(radius));  
}

}  // namespace func
}  // namespace gal

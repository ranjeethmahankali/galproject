#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(boundingCircle,
         "Creates a bounding circle for the given points. The 3d points are "
         "flattened to 2d by removing the z-coordinate.",
         (((data::ReadView<glm::vec3, 1>), points, "Points")),
         ((gal::Circle2d, circle, "Bounding circle"),
          (glm::vec2, center, "Center of the circle"),
          (float, radius, "Radius of the circle")))
{
  // TODO: Clean this up to consume 2d points as inputs directly.
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

GAL_FUNC(circle2d,
         "Creates a bounding circle for the given points. The 3d points are "
         "flattened to 2d by removing the z-coordinate.",
         ((glm::vec2, center, "Center"), (float, radius, "Radius")),
         ((gal::Circle2d, circle, "Circle")))
{
  circle.center(center);
  circle.radius(radius);
}

GAL_FUNC(circle2dFromDiameter,
         "Creates a 2d circle with the given points as the ends of the diameter",
         ((glm::vec2, pt1, "First point"), (glm::vec2, pt2, "Second point")),
         ((gal::Circle2d, circle, "Circle"),
          (glm::vec2, center, "Center of the circle"),
          (float, radius, "Radius of the circle")))
{
  circle = gal::Circle2d::createFromDiameter(pt1, pt2);
  center = circle.center();
  radius = circle.radius();
}

GAL_FUNC(circumCircle2d,
         "Creates a 2d circle with the given points as the ends of the diameter",
         ((glm::vec2, pt1, "First point"),
          (glm::vec2, pt2, "Second point"),
          (glm::vec2, pt3, "Third point")),
         ((gal::Circle2d, circle, "Circle"),
          (glm::vec2, center, "Center of the circle"),
          (float, radius, "Radius of the circle")))
{
  circle = gal::Circle2d::createCircumcircle(pt1, pt2, pt3);
  center = circle.center();
  radius = circle.radius();
}

GAL_FUNC(bounds,
         "Bounding box of the circle",
         ((gal::Circle2d, circ, "The circle")),
         ((gal::Box2, bbox, "Bounding box")))
{
  bbox = circ.bounds();
}

GAL_FUNC(area,
         "Area of the circle",
         ((gal::Circle2d, circ, "The circle")),
         ((float, result, "Area")))
{
  result = circ.area();
}

GAL_FUNC(perimeter,
         "Perimter of the circle",
         ((gal::Circle2d, circ, "The circle")),
         ((float, result, "Perimeter")))
{
  result = circ.perimeter();
}

void bind_CircleFunc()
{
  GAL_FN_BIND(
    bounds, area, boundingCircle, circle2d, circle2dFromDiameter, circumCircle2d);
}

}  // namespace func
}  // namespace gal

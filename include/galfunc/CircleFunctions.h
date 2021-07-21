#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(boundingCircle,
              1,
              3,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              ((gal::PointCloud, points, "Points")),
              ((gal::Circle2d, circle, "Bounding circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")));

GAL_FUNC_DECL(circle2d,
              2,
              1,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              ((glm::vec2, center, "Center"), (float, radius, "Radius")),
              ((gal::Circle2d, circle, "Circle")));

GAL_FUNC_DECL(circle2dFromDiameter,
              2,
              3,
              "Creates a 2d circle with the given points as the ends of the diameter",
              ((glm::vec2, pt1, "First point"), (glm::vec2, pt2, "Second point")),
              ((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")));

GAL_FUNC_DECL(circumCircle2d,
              3,
              3,
              "Creates a 2d circle with the given points as the ends of the diameter",
              ((glm::vec2, pt1, "First point"),
               (glm::vec2, pt2, "Second point"),
               (glm::vec2, pt3, "Third point")),
              ((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")));

}  // namespace func
}  // namespace gal

#define GAL_CircleFunctions boundingCircle, circle2d, circle2dFromDiameter, circumCircle2d

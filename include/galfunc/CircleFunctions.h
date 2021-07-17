#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(((gal::Circle2d, circle, "Bounding circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              boundingCircle,
              true,
              1,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (gal::PointCloud, points, "Points"));

GAL_FUNC_DECL(((gal::Circle2d, circle, "Circle")),
              circle2d,
              true,
              2,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (glm::vec2, center, "Center"),
              (float, radius, "Radius"));

GAL_FUNC_DECL(((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              circle2dFromDiameter,
              true,
              2,
              "Creates a 2d circle with the given points as the ends of the diameter",
              (glm::vec2, pt1, "First point"),
              (glm::vec2, pt2, "Second point"));

GAL_FUNC_DECL(((gal::Circle2d, circle, "Circle"),
               (glm::vec2, center, "Center of the circle"),
               (float, radius, "Radius of the circle")),
              circumCircle2d,
              true,
              3,
              "Creates a 2d circle with the given points as the ends of the diameter",
              (glm::vec2, pt1, "First point"),
              (glm::vec2, pt2, "Second point"),
              (glm::vec2, pt3, "Third point"));

}  // namespace func
}  // namespace gal

#define GAL_CircleFunctions boundingCircle, circle2d, circle2dFromDiameter, circumCircle2d

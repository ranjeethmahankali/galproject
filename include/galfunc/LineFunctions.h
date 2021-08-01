#pragma once

#include <galcore/Line.h>
#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(line2d,
              2,
              1,
              "Creates a 2d line from the given 2d points.",
              ((glm::vec2, start, "First point of the line."),
               (glm::vec2, end, "Second point of the line.")),
              ((gal::Line2d, line, "The line.")));

GAL_FUNC_DECL(line3d,
              2,
              1,
              "Creates a 3d line from the given 3d points.",
              ((glm::vec3, start, "First point of the line."),
               (glm::vec3, end, "Second point of the line.")),
              ((gal::Line3d, line, "The line.")));

GAL_FUNC_DECL(
  samplePointsOnLine2d,
  2,
  1,
  "Samples the given number of points evenly on the line, including the end points.",
  ((gal::Line2d, line, "The line"), (int32_t, nPts, "The number of points to sample")),
  ((std::vector<glm::vec2>, points, "The sampled points")));

}  // namespace func
}  // namespace gal

#define GAL_LineFunctions line2d, line3d, samplePointsOnLine2d

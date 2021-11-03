#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(line2d,
         "Creates a 2d line from the given 2d points.",
         ((glm::vec2, start, "First point of the line."),
          (glm::vec2, end, "Second point of the line.")),
         ((gal::Line2d, line, "The line.")))
{
  line.mStart = start;
  line.mEnd   = end;
}

GAL_FUNC(line3d,
         "Creates a 3d line from the given 3d points.",
         ((glm::vec3, start, "First point of the line."),
          (glm::vec3, end, "Second point of the line.")),
         ((gal::Line3d, line, "The line.")))
{
  line.mStart = start;
  line.mEnd   = end;
}

GAL_FUNC(
  samplePointsOnLine2d,
  "Samples the given number of points evenly on the line, including the end points.",
  ((gal::Line2d, line, "The line"), (int32_t, nPts, "The number of points to sample")),
  (((data::WriteView<glm::vec2, 1>), points, "The sampled points")))
{
  if (nPts < 2) {
    throw std::out_of_range("Invalid point count");
  }
  points.reserve(nPts);
  glm::vec2 pt   = line.mStart;
  glm::vec2 step = line.vec() / float((nPts)-1);
  for (int32_t i = 0; i < nPts; i++) {
    points.push_back(pt);
    pt += step;
  }
}

GAL_FUNC(
  samplePointsOnLine3d,
  "Samples the given number of points evenly on the line, including the end points.",
  ((gal::Line3d, line, "The line"), (int32_t, nPts, "The number of points to sample")),
  (((data::WriteView<glm::vec3, 1>), points, "The sampled points")))
{
  if (nPts < 2) {
    throw std::out_of_range("Invalid point count");
  }
  points.reserve(nPts);
  glm::vec3 pt   = line.mStart;
  glm::vec3 step = line.vec() / float((nPts)-1);
  for (int32_t i = 0; i < nPts; i++) {
    points.push_back(pt);
    pt += step;
  }
}

void bind_LineFunc()
{
  GAL_FN_BIND(line2d, line3d);
  GAL_FN_BIND_OVERLOADS(pointsOnLine, samplePointsOnLine2d, samplePointsOnLine3d);
}

}  // namespace func
}  // namespace gal

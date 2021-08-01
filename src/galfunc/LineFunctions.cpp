#include <galfunc/LineFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(line2d,
              2,
              1,
              "Creates a 2d line from the given 2d points.",
              ((glm::vec2, start, "First point of the line."),
               (glm::vec2, end, "Second point of the line.")),
              ((gal::Line2d, line, "The line.")))
{
  line->mStart = *start;
  line->mEnd   = *end;
}

GAL_FUNC_DEFN(line3d,
              2,
              1,
              "Creates a 3d line from the given 3d points.",
              ((glm::vec3, start, "First point of the line."),
               (glm::vec3, end, "Second point of the line.")),
              ((gal::Line3d, line, "The line.")))
{
  line->mStart = *start;
  line->mEnd   = *end;
}

GAL_FUNC_DEFN(
  samplePointsOnLine2d,
  2,
  1,
  "Samples the given number of points evenly on the line, including the end points.",
  ((gal::Line2d, line, "The line"), (int32_t, nPts, "The number of points to sample")),
  ((std::vector<glm::vec2>, points, "The sampled points")))
{
  if (*nPts < 2) {
    throw std::out_of_range("Invalid point count");
  }
  points->resize(*nPts);
  auto      pbegin = points->begin();
  auto      pend   = points->end();
  glm::vec2 pt     = line->mStart;
  glm::vec2 step   = line->vec() / float((*nPts) - 1);
  while (pbegin != pend) {
    *(pbegin++) = pt;
    pt += step;
  }
}

GAL_FUNC_DEFN(
  samplePointsOnLine3d,
  2,
  1,
  "Samples the given number of points evenly on the line, including the end points.",
  ((gal::Line3d, line, "The line"), (int32_t, nPts, "The number of points to sample")),
  ((std::vector<glm::vec3>, points, "The sampled points")))
{
  if (*nPts < 2) {
    throw std::out_of_range("Invalid point count");
  }
  points->resize(*nPts);
  auto      pbegin = points->begin();
  auto      pend   = points->end();
  glm::vec3 pt     = line->mStart;
  glm::vec3 step   = line->vec() / float((*nPts) - 1);
  while (pbegin != pend) {
    *(pbegin++) = pt;
    pt += step;
  }
}

}  // namespace func
}  // namespace gal

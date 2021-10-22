#include <galfunc/LineFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(line2d, ((glm::vec2, start), (glm::vec2, end)), ((gal::Line2d, line)))
{
  line.mStart = start;
  line.mEnd   = end;
}

GAL_FUNC_DEFN(line3d, ((glm::vec3, start), (glm::vec3, end)), ((gal::Line3d, line)))
{
  line.mStart = start;
  line.mEnd   = end;
}

GAL_FUNC_DEFN(samplePointsOnLine2d,
              ((gal::Line2d, line), (int32_t, nPts)),
              (((data::WriteView<glm::vec2, 1>), points)))
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

GAL_FUNC_DEFN(samplePointsOnLine3d,
              ((gal::Line3d, line), (int32_t, nPts)),
              (((data::WriteView<glm::vec3, 1>), points)))
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

}  // namespace func
}  // namespace gal

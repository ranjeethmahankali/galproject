#include <galcore/Line.h>

namespace gal {

Box2 Line2d::bounds() const
{
  return Box2(mStart, mEnd);
}

glm::vec2 Line2d::vec() const
{
  return mEnd - mStart;
}

Box3 Line3d::bounds() const
{
  return Box3(mStart, mEnd);
}

glm::vec3 Line3d::vec() const
{
  return mEnd - mStart;
}

}  // namespace gal

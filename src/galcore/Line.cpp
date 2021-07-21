#include <galcore/Line.h>

namespace gal {

Box2 Line2d::bounds() const
{
  return Box2(mStart, mEnd);
}

Box3 Line3d::bounds() const
{
  return Box3(mStart, mEnd);
}

}  // namespace gal

#include <galcore/Line.h>

namespace gal {

Box2 Line2d::bounds() const
{
  return Box2(mStart, mEnd);
}

}  // namespace gal
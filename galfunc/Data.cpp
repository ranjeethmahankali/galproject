#include <galfunc/Data.h>

namespace gal {
Bool::Bool(bool b)
    : mValue(b) {};

Bool::operator bool() const
{
  return mValue;
}

Bool::operator bool&()
{
  return mValue;
}

}  // namespace gal

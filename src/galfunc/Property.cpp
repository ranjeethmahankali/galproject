#include <galfunc/Property.h>

namespace gal {
namespace func {

int Properties::add(IPropData* ptr)
{
  int i = int(mProps.size());
  mProps.push_back(ptr);
  return i;
}

void Properties::remove(int i)
{
  if (i > -1 && i < mProps.size()) {
    mProps[i] = nullptr;
  }
}

}  // namespace func
}  // namespace gal

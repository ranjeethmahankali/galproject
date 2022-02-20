#include <galfunc/Property.h>

namespace gal {
namespace func {

int Properties::add(IProperty* ptr)
{
  ptr->resize(size());
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

void Properties::clear()
{
  for (auto p : mProps) {
    p->clear();
  }
}

void Properties::reserve(size_t n)
{
  for (auto p : mProps) {
    p->reserve(n);
  }
}

void Properties::resize(size_t n)
{
  for (auto p : mProps) {
    p->resize(n);
  }
}

void Properties::swap(size_t i, size_t j)
{
  for (auto p : mProps) {
    p->swap(i, j);
  }
}

size_t Properties::size() const
{
  if (mProps.empty()) {
    return 0;
  }
  else {
    return mProps.back()->size();
  }
}

}  // namespace func
}  // namespace gal

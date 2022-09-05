#include <iostream>

#include <Functions.h>
#include <Property.h>

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
    if (p) {
      p->clear();
    }
  }
}

void Properties::reserve(size_t n)
{
  for (auto p : mProps) {
    if (p) {
      p->reserve(n);
    }
  }
}

void Properties::resize(size_t n)
{
  for (auto p : mProps) {
    if (p) {
      p->resize(n);
    }
  }
}

void Properties::swap(size_t i, size_t j)
{
  for (auto p : mProps) {
    if (p) {
      p->swap(i, j);
    }
  }
}

size_t Properties::size() const
{
  if (mProps.empty()) {
    return 0;
  }
  else if (mProps.back()) {
    return mProps.back()->size();
  }
  else {
    return 0;
  }
}

}  // namespace func
}  // namespace gal

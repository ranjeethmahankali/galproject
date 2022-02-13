#pragma once
#include <utility>
#include <vector>

namespace gal {
namespace func {
namespace graph {

struct IPropData
{
  virtual void reserve(size_t n) = 0;
  virtual void resize(size_t n)  = 0;
  virtual void clear()           = 0;
};

template<typename T>
struct PropData : public IPropData, public std::vector<T>
{
};

template<typename T>
struct Property
{
  int index = -1;

public:
  Property()
  {
    // TODO: Add a new propdata to the store and capture index and reference to propdata.
  }
  ~Property()
  {
    // TODO: delete property from the store using the index.
  }
  // Forbid copy.
  Property(const Property&) = delete;
  const Property& operator=(const Property&) = delete;
  // Move semantics.
  const Property& operator=(Property&& other)
  {
    if (index == other.index && this == &other) {
      return;
    }
    if (index != -1) {
      // TODO: delete this property from the store using the index.
    }
    index = std::exchange(other.index, -1);
  }
  Property(Property&& other) { *this = other; }
};

}  // namespace graph
}  // namespace func
}  // namespace gal

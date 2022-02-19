#pragma once
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace gal {
namespace func {

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

struct Properties
{
  int  add(IPropData* ptr);
  void remove(int idx);

private:
  std::vector<IPropData*> mProps;
};

template<typename T>
struct Property
{
private:
  std::unique_ptr<IPropData> mData;
  Properties&                mContainer;
  int                        mIndex = -1;

public:
  int index() const { return mIndex; }

  int& index() { return mIndex; }

  Property(Properties& container)
      : mData(std::make_unique<PropData<T>>())
      , mContainer(container)
      , mIndex(container.add(mData.get()))
  {}

  ~Property() { mData.reset(); }
  // Forbid copy.
  Property(const Property&) = delete;
  const Property& operator=(const Property&) = delete;
  // Move semantics.
  const Property& operator=(Property&& other)
  {
    if (mIndex == other.index() && this == &other) {
      return *this;
    }
    mContainer.remove(mIndex);
    mIndex = std::exchange(other.index(), -1);
    return *this;
  }
  Property(Property&& other) { *this = other; }
};

}  // namespace func
}  // namespace gal

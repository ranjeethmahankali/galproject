#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace gal {
namespace func {

/**
 * @brief Property interface.
 *
 */
struct IProperty
{
  virtual void   reserve(size_t n)        = 0;
  virtual void   resize(size_t n)         = 0;
  virtual void   clear()                  = 0;
  virtual void   swap(size_t i, size_t j) = 0;
  virtual size_t size() const             = 0;

  virtual ~IProperty() = default;
};

/**
 * @brief Container for all properties that apply to the same set of elements.
 * The properties within will all be of the same size, equal to the number of elements.
 */
struct Properties
{
  int    add(IProperty* ptr);
  void   remove(int idx);
  void   clear();
  void   reserve(size_t n);
  void   resize(size_t n);
  void   swap(size_t i, size_t j);
  size_t size() const;

private:
  std::vector<IProperty*> mProps;
};

/**
 * @brief Property of give type.
 *
 * @tparam T
 */
template<typename T>
struct Property : public IProperty
{
private:
  std::vector<T> mData;
  Properties*    mContainer = nullptr;
  int            mIndex     = -1;

  template<typename U>
  static size_t getIndex(const U& item)
  {
    if constexpr (std::is_convertible_v<U, size_t>) {
      return size_t(item);
    }
    else {
      return size_t(item.index());
    }
  }

  void removeFromContainer()
  {
    if (mContainer) {
      mContainer->remove(mIndex);
    }
  }

public:
  int index() const { return mIndex; }

  int& index() { return mIndex; }

  explicit Property(Properties& container)
      : mContainer(&container)
      , mIndex(container.add(this))
  {}

  Property() = default;

  // Forbid copy.
  Property(const Property&)                  = delete;
  const Property& operator=(const Property&) = delete;
  // Move semantics.
  const Property& operator=(Property&& other)
  {
    if (this == &other) {
      return *this;
    }
    removeFromContainer();
    mData      = std::move(other.mData);
    mContainer = std::exchange(other.mContainer, nullptr);
    mIndex     = std::exchange(other.index(), -1);
    return *this;
  }
  Property(Property&& other) { *this = std::move(other); }

  virtual ~Property() { removeFromContainer(); }

  void reserve(size_t n) override { mData.reserve(n); }

  void resize(size_t n) override { mData.resize(n); }

  void clear() override { mData.clear(); }

  void swap(size_t i, size_t j) override { std::swap(mData[i], mData[j]); }

  size_t size() const override { return mData.size(); }

  template<typename U>
  T& operator[](const U& item)
  {
    return mData[getIndex(item)];
  }

  template<typename U>
  const T& operator[](const U& item) const
  {
    return mData[getIndex(item)];
  }
};

}  // namespace func
}  // namespace gal

#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace gal {
namespace func {

struct IProperty
{
  virtual void   reserve(size_t n)        = 0;
  virtual void   resize(size_t n)         = 0;
  virtual void   clear()                  = 0;
  virtual void   swap(size_t i, size_t j) = 0;
  virtual size_t size() const             = 0;
};

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

template<typename T>
struct Property : public IProperty, private std::vector<T>
{
private:
  Properties* mContainer;
  int         mIndex = -1;

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

public:
  int index() const { return mIndex; }

  int& index() { return mIndex; }

  Property(Properties& container)
      : mContainer(&container)
      , mIndex(container.add(this))
  {}

  // Forbid copy.
  Property(const Property&) = delete;
  const Property& operator=(const Property&) = delete;
  // Move semantics.
  const Property& operator=(Property&& other)
  {
    if (mIndex == other.index() && this == &other) {
      return *this;
    }
    mContainer->remove(mIndex);
    mIndex = std::exchange(other.index(), -1);
    return *this;
  }
  Property(Property&& other) { *this = std::move(other); }

  void reserve(size_t n) override { std::vector<T>::reserve(n); }

  void resize(size_t n) override { std::vector<T>::resize(n); }

  void clear() override { std::vector<T>::clear(); }

  void swap(size_t i, size_t j) override { std::swap(this->at(i), this->at(j)); }

  size_t size() const override { return std::vector<T>::size(); }

  template<typename U>
  T& operator[](const U& item)
  {
    return std::vector<T>::at(getIndex(item));
  }

  template<typename U>
  const T& operator[](const U& item) const
  {
    return std::vector<T>::at(getIndex(item));
  }
};

}  // namespace func
}  // namespace gal

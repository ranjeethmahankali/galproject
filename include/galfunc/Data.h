#pragma once

#include <stdint.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#include <galcore/Traits.h>
#include <galcore/Types.h>

namespace gal {
namespace func {

template<typename T>
struct DataTree
{
  using DepthT     = uint8_t;  // Max is 255.
  using ValueType  = std::conditional_t<std::is_polymorphic_v<T>, std::shared_ptr<T>, T>;
  using value_type = ValueType;  // To support stl helper functions.
  struct InstanceT
  {
    ValueType mValue;
    DepthT    mDepth;

    InstanceT(DepthT d, ValueType val)
        : mValue(std::move(val))
        , mDepth(d)
    {}

    InstanceT(ValueType val)
        : InstanceT(0, std::move(val))
    {}

    template<typename... TArgs>
    InstanceT(DepthT d, TArgs... args)
        : mValue(args...)
        , mDepth(d)
    {}

    operator T&() { return mValue; }
    operator const T&() const { return mValue; }
  };

  using InternalStorageT = std::vector<InstanceT>;

  template<DepthT Dim>
  struct Iterator;

  template<uint32_t Dim>
  struct ReadView;

  std::vector<InstanceT> mValues;

  DepthT depth() const
  {
    if (mValues.empty()) {
      return 0;
    }
    else {
      return mValues[0].mDepth;
    }
  };

  void ensureDepth(DepthT d)
  {
    if (!mValues.empty()) {
      mValues[0].mDepth = std::max(d, mValues[0].mDepth);
    }
  }

  void push_back(DepthT d, T item)
  {
    ensureDepth(d);
    mValues.emplace_back(d, std::move(item));
  }

  void push_back(T item) { push_back(0, std::move(item)); }

  template<typename... TArgs>
  void emplace_back(DepthT d, TArgs... args)
  {
    ensureDepth(d);
    mValues.emplace_back(d, args...);
  }

  void reserve(size_t n) { mValues.reserve(n); }

  size_t size() const { return mValues.size(); }
};

template<typename T, size_t Dim>
struct Dereferenced
{
  using Type = const typename DataTree<T>::template ReadView<Dim>;
};

template<typename T>
struct Dereferenced<T, 0>
{
  using Type = const typename DataTree<T>::ValueType&;
};

template<typename T>
template<uint32_t Dim>
struct DataTree<T>::ReadView
{
  using DepthT = typename DataTree<T>::DepthT;
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

  template<DepthT D2>
  using Iterator = typename DataTree<T>::template Iterator<D2>;

  const DataTree<T>& mTree;
  size_t             mStart;

  ReadView(const DataTree<T>& src)
      : mTree(src)
      , mStart(0)
  {}

  ReadView(Iterator<Dim>& iter)
      : mTree(iter.mTree)
      , mStart(iter.mIndex)
  {}

  const typename DataTree<T>::InternalStorageT& internalStorage() const
  {
    return mTree.mValues;
  }

  Iterator<Dim - 1> end() const
  {
    return Iterator<Dim - 1>(mTree, internalStorage().size());
  }

  Iterator<Dim - 1> begin() const { return Iterator<Dim - 1>(mTree, mStart); }

  typename Iterator<Dim - 1>::DereferenceT operator[](size_t i) { return *(begin() + i); }

  friend std::ostream& operator<<(std::ostream&                                os,
                                  const gal::func::DataTree<T>::ReadView<Dim>& view)
  {
    os << '(' << gal::TypeInfo<T>::name() << ' ' << Dim << "d view)";
    return os;
  }
};

template<typename T>
template<typename DataTree<T>::DepthT Dim>
struct DataTree<T>::Iterator
{
  using DereferenceT = typename Dereferenced<T, Dim>::Type;

  const DataTree<T>& mTree;
  size_t             mIndex;

  Iterator(const DataTree<T>& tree, size_t index)
      : mTree(tree)
      , mIndex(index)
  {}

  const DataTree<T>::InternalStorageT& internalStorage() const { return mTree.mValues; }

  const DataTree<T>::InternalStorageT* internalPtr() const { return &(mTree.mValues); }

  bool operator==(const Iterator& other) const
  {
    return internalPtr() == other.internalPtr() && mIndex == other.mIndex;
  }

  template<DepthT D2>
  bool operator==(const DataTree<T>::Iterator<D2>& other)
  {
    return internalPtr() == other.internalPtr() && mIndex == other.mIndex &&
           (Dim == D2 || mIndex == internalStorage().size());
  }

  bool operator!=(const Iterator& other) const { return !(*this == other); }

  const Iterator& operator++()
  {
    if constexpr (Dim == 0) {
      ++mIndex;
    }
    else {
      do {
        ++mIndex;
      } while (mIndex < internalStorage().size() &&
               internalStorage()[mIndex].mDepth < Dim);
    }
    if (internalStorage()[mIndex].mDepth > Dim) {
      mIndex = internalStorage().size();
    }
    return *this;
  }

  Iterator operator++(int)
  {
    Iterator result = *this;
    ++(*this);
    return result;
  }

  Iterator operator+(size_t offset)
  {
    Iterator result = *this;
    for (size_t i = 0; i < offset && result.mIndex < internalStorage().size(); i++) {
      ++result;
    }
    return result;
  }

  Iterator operator+=(size_t offset) { *this = *this + offset; }

  DereferenceT operator*()
  {
    if constexpr (Dim == 0) {
      return internalStorage()[mIndex].mValue;
    }
    else {
      return DataTree<T>::ReadView<Dim>(*this);
    }
  }
};

}  // namespace func
}  // namespace gal

namespace std {

template<typename T>
std::ostream& operator<<(std::ostream& os, const gal::func::DataTree<T>& tree)
{
  using namespace gal::func;
  size_t dmax = size_t(tree.depth());
  for (const typename DataTree<T>::InstanceT& i : tree.mValues) {
    for (uint32_t d = 0; d < dmax; d++) {
      if (dmax - d > i.mDepth) {
        os << ' ';
      }
      else {
        os << '*';
      }
    }
    os << ' ';
    if constexpr (gal::IsPrintable<T>::value) {
      os << i.mValue;
    }
    else {
      os << '<' << gal::TypeInfo<T>::name() << '>';
    }
    os << std::endl;
  }
  return os;
}

}  // namespace std

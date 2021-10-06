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
  using DepthT    = uint32_t;
  using ValueType = std::conditional_t<std::is_polymorphic_v<T>, std::shared_ptr<T>, T>;
  struct InstanceT
  {
    ValueType mValue;
    DepthT    mDepth;

    InstanceT(DepthT d, ValueType val)
        : mValue(std::move(val))
        , mDepth(d)
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

  template<typename... TArgs>
  void emplace_back(DepthT d, TArgs... args)
  {
    ensureDepth(d);
    mValues.emplace_back(d, args...);
  }

  void reserve(size_t n) { mValues.reserve(n); }

  size_t size() const { return mValues.size(); }
};

template<typename T, uint32_t Dim>
struct DataView
{
  using DepthT = typename DataTree<T>::DepthT;
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

  template<DepthT D2>
  using Iterator = typename DataTree<T>::template Iterator<D2>;

  DataTree<T>& mTree;
  size_t       mStart;

  DataView(DataTree<T>& src)
      : mTree(src)
      , mStart(0)
  {}

  DataView(Iterator<Dim>& iter)
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

  Iterator<Dim - 1> begin()
  {
    if (Dim > mTree.depth()) {
      return end();
    }
    return Iterator<Dim - 1>(mTree, mStart);
  }

  friend std::ostream& operator<<(std::ostream&                      os,
                                  const gal::func::DataView<T, Dim>& view)
  {
    os << '(' << gal::TypeInfo<T>::name() << ' ' << Dim << "d view)";
    return os;
  }
};

template<typename T, size_t Dim>
struct Dereferenced
{
  using Type = DataView<T, Dim>;
};

template<typename T>
struct Dereferenced<T, 0>
{
  using Type = typename DataTree<T>::ValueType&;
};

template<typename T>
template<uint32_t Dim>
struct DataTree<T>::Iterator
{
  using DereferenceT = typename Dereferenced<T, Dim>::Type;

  DataTree<T>& mTree;
  size_t       mIndex;

  Iterator(DataTree<T>& tree, size_t index)
      : mTree(tree)
      , mIndex(index)
  {}

  DataTree<T>::InternalStorageT&       internalStorage() { return mTree.mValues; }
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

  DereferenceT operator*()
  {
    if constexpr (Dim == 0) {
      return internalStorage()[mIndex].mValue;
    }
    else {
      return DataView<T, Dim>(*this);
    }
  }

  const DereferenceT operator*() const
  {
    if constexpr (Dim == 0) {
      return internalStorage()[mIndex].mValue;
    }
    else {
      return DataView<T, Dim>(*this);
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

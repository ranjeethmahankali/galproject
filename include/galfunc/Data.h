#pragma once

#include <stdint.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <galcore/Traits.h>
#include <galcore/Types.h>
#include <tbb/tbb.h>

namespace gal {
namespace func {
namespace data {

using DepthT = uint8_t;  // Max is 255.

template<typename T>
class DataTree
{
public:
  using ValueType  = std::conditional_t<std::is_polymorphic_v<T>, std::shared_ptr<T>, T>;
  using value_type = ValueType;  // To support stl helper functions.

private:
  using InternalStorageT = std::vector<ValueType>;

  template<typename U, DepthT Dim>
  friend struct Iterator;

  template<typename U, DepthT Dim>
  friend struct InputView;

  template<typename U>
  friend struct OutputView;

  InternalStorageT    mValues;
  std::vector<DepthT> mDepths;

  void ensureDepth(DepthT d)
  {
    if (!mDepths.empty()) {
      mDepths[0] = std::max(d, mDepths[0]);
    }
  }

  void reserve(size_t n)
  {
    mValues.reserve(n);
    mDepths.reserve(n);
  }

  void push_back(DepthT d, T item)
  {
    ensureDepth(d);
    mValues.emplace_back(std::move(item));
    mDepths.push_back(d);
  }

  void push_back(T item) { push_back(0, std::move(item)); }

  template<typename... TArgs>
  void emplace_back(DepthT d, TArgs... args)
  {
    ensureDepth(d);
    mValues.emplace_back(args...);
    mDepths.push_back(d);
  }

public:
  DepthT maxDepth() const
  {
    if (mDepths.empty()) {
      return 0;
    }
    else {
      return mDepths[0];
    }
  };

  DepthT&       depth(size_t i) { return mDepths[i]; }
  const DepthT& depth(size_t i) const { return mDepths[i]; }

  ValueType&       value(size_t i) { return mValues[i]; }
  const ValueType& value(size_t i) const { return mValues[i]; }

  size_t size() const { return mValues.size(); }

  friend std::ostream& operator<<(std::ostream& os, const DataTree<T>& tree)
  {
    using namespace gal::func::data;
    size_t dmax = size_t(tree.maxDepth());
    for (size_t i = 0; i < tree.size(); i++) {
      for (uint32_t d = 0; d < dmax; d++) {
        if (dmax - d > tree.mDepths[i]) {
          os << ' ';
        }
        else {
          os << '*';
        }
      }
      os << ' ';
      if constexpr (gal::IsPrintable<T>::value) {
        os << tree.mValues[i];
      }
      else {
        os << '<' << gal::TypeInfo<T>::name() << '>';
      }
      os << std::endl;
    }
    return os;
  }
};

template<typename U, DepthT Dim>
struct Iterator;

template<typename U, DepthT Dim>
struct InputView;

template<typename T, size_t Dim>
struct Dereferenced
{
  using Type = InputView<T, Dim>;
};

template<typename T>
struct Dereferenced<T, 0>
{
  using Type = const typename DataTree<T>::ValueType&;
};

template<typename T, DepthT Dim>
struct InputView
{
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

  const DataTree<T>& mTree;
  size_t             mStart;

  InputView(const DataTree<T>& src)
      : mTree(src)
      , mStart(0)
  {}

  InputView(Iterator<T, Dim>& iter)
      : mTree(iter.mTree)
      , mStart(iter.mIndex)
  {}

  const typename DataTree<T>::InternalStorageT& internalStorage() const
  {
    return mTree.mValues;
  }

  Iterator<T, Dim - 1> end() const
  {
    return Iterator<T, Dim - 1>(mTree, internalStorage().size());
  }

  Iterator<T, Dim - 1> begin() const { return Iterator<T, Dim - 1>(mTree, mStart); }

  typename Iterator<T, Dim - 1>::DereferenceT operator[](size_t i)
  {
    return *(begin() + i);
  }

  friend std::ostream& operator<<(std::ostream& os, const InputView<T, Dim>& view)
  {
    os << '(' << gal::TypeInfo<T>::name() << ' ' << Dim << "d view)";
    return os;
  }
};

template<typename T>
struct OutputView
{
private:
  DataTree<T>& mTree;
  DepthT       mDepth;

public:
  using value_type = typename DataTree<T>::value_type;
  OutputView(DataTree<T>& tree)
      : mTree(tree) {};

  ~OutputView() {}

  OutputView(const OutputView&) = delete;
  OutputView(OutputView&&)      = delete;
  const OutputView& operator=(const OutputView&) = delete;
  const OutputView& operator=(OutputView&&) = delete;

  void push_back(DepthT depth, T item) { mTree.push_back(depth, std::move(item)); }

  void push_back(T item) { push_back(0, std::move(item)); }

  void reserve(size_t n) { mTree.reserve(n); }
};

template<typename T, DepthT Dim>
struct Iterator
{
  using DereferenceT     = typename Dereferenced<T, Dim>::Type;
  using InternalStorageT = typename DataTree<T>::InternalStorageT;

  const DataTree<T>& mTree;
  size_t             mIndex;

  Iterator(const DataTree<T>& tree, size_t index)
      : mTree(tree)
      , mIndex(index)
  {}

  const InternalStorageT&    storage() const { return mTree.mValues; }
  const std::vector<DepthT>& depths() const { return mTree.mDepths; }
  const InternalStorageT*    internalPtr() const { return &(mTree.mValues); }

  bool operator==(const Iterator& other) const
  {
    return internalPtr() == other.internalPtr() && mIndex == other.mIndex;
  }

  template<DepthT D2>
  bool operator==(const Iterator<T, D2>& other)
  {
    return internalPtr() == other.internalPtr() && mIndex == other.mIndex &&
           (Dim == D2 || mIndex == storage().size());
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
      } while (mIndex < storage().size() && depths()[mIndex] < Dim);
    }
    if (depths()[mIndex] > Dim) {
      mIndex = storage().size();
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
    for (size_t i = 0; i < offset && result.mIndex < storage().size(); i++) {
      ++result;
    }
    return result;
  }

  Iterator operator+=(size_t offset) { *this = *this + offset; }

  DereferenceT operator*()
  {
    if constexpr (Dim == 0) {
      return storage()[mIndex];
    }
    else {
      return InputView<T, Dim>(*this);
    }
  }
};

}  // namespace data
}  // namespace func
}  // namespace gal

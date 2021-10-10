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
  friend struct ReadView;

  template<typename U, DepthT Dim>
  friend struct WriteViewBase;

  template<typename U, DepthT Dim>
  friend struct WriteView;

  InternalStorageT                         mValues;
  std::vector<DepthT>                      mDepths;
  std::vector<DepthT>                      mQueuedDepths;
  mutable std::vector<std::vector<size_t>> mCache;
  mutable int32_t                          mAccessFlag   = 0;
  mutable bool                             mIsCacheValid = false;

  void ensureDepth(DepthT d)
  {
    if (!mDepths.empty()) {
      mDepths[0] = std::max(++d, mDepths[0]);
    }
  }

  void pushDepth(DepthT d)
  {
    if (mQueuedDepths.empty()) {
      mDepths.push_back(d);
    }
    else {
      mDepths.push_back(mQueuedDepths.back());
      mQueuedDepths.clear();
    }
  }

  void queueDepth(DepthT d)
  {
    auto match = std::upper_bound(mQueuedDepths.begin(), mQueuedDepths.end(), d);
    if (match == mQueuedDepths.end()) {
      mQueuedDepths.push_back(d);
    }
    else if (*match == d) {
      throw std::invalid_argument(
        "This depth cannot be queued because it has already been queued.");
    }
    else {
      mQueuedDepths.insert(match, d);
    }
  }

  void unqueueDepth(DepthT d)
  {
    mQueuedDepths.erase(std::remove(mQueuedDepths.begin(), mQueuedDepths.end(), d),
                        mQueuedDepths.end());
  }

  void ensureCache() const
  {
    if (mIsCacheValid) {
      return;
    }
    mCache.clear();
    mCache.resize(maxDepth());
    for (size_t i = 0; i < mValues.size(); i++) {
      for (DepthT d = 0; d < mDepths[i]; d++) {
        mCache[d].push_back(i);
      }
    }
    mIsCacheValid = true;
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

  void reserve(size_t n)
  {
    mValues.reserve(n);
    mDepths.reserve(n);
  }

  void push_back(DepthT d, T item)
  {
    ensureDepth(d);
    mValues.emplace_back(std::move(item));
    pushDepth(d);
  }

  template<typename... TArgs>
  void emplace_back(DepthT d, TArgs... args)
  {
    ensureDepth(d);
    mValues.emplace_back(args...);
    pushDepth(d);
  }

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
struct ReadView;

template<typename T, size_t Dim>
struct Dereferenced
{
  using Type = ReadView<T, Dim>;
};

template<typename T>
struct Dereferenced<T, 0>
{
  using Type = const typename DataTree<T>::ValueType&;
};

template<typename T, DepthT Dim>
struct ReadView
{
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

private:
  const DataTree<T>& mTree;
  size_t             mPos;

  void setReadMode()
  {
    if (mTree.mAccessFlag > 0) {
      throw std::invalid_argument(
        "Cannot create a read-view because the data-tree has at least one active "
        "write-view");
    }
    mTree.ensureCache();
    mTree.mAccessFlag--;
  }

  size_t startIndex() const
  {
    if constexpr (Dim == 0) {
      return mPos;
    }
    else {
      return mPos == storage().size() ? mPos : mTree.mCache[Dim - 1][mPos];
    }
  }

public:
  ReadView(const DataTree<T>& src)
      : mTree(src)
      , mPos(0)
  {
    setReadMode();
  }

  ReadView(Iterator<T, Dim>& iter)
      : mTree(iter.mTree)
      , mPos(iter.mPos)
  {
    setReadMode();
  }

  ~ReadView() { mTree.mAccessFlag++; }

  ReadView(const ReadView&) = delete;
  ReadView(ReadView&&)      = delete;
  const ReadView& operator=(const ReadView&) = delete;
  const ReadView& operator=(ReadView&&) = delete;

  const typename DataTree<T>::InternalStorageT& storage() const { return mTree.mValues; }

  Iterator<T, Dim - 1> end() const
  {
    return Iterator<T, Dim - 1>(mTree, storage().size());
  }

  Iterator<T, Dim - 1> begin() const { return Iterator<T, Dim - 1>(mTree, startIndex()); }

  typename Iterator<T, Dim - 1>::DereferenceT operator[](size_t i)
  {
    return *(begin() + i);
  }

  friend std::ostream& operator<<(std::ostream& os, const ReadView<T, Dim>& view)
  {
    os << '(' << gal::TypeInfo<T>::name() << ' ' << Dim << "d view)";
    return os;
  }
};

template<typename T, DepthT Dim>
struct Iterator
{
  using DereferenceT     = typename Dereferenced<T, Dim>::Type;
  using InternalStorageT = typename DataTree<T>::InternalStorageT;

  const DataTree<T>& mTree;
  size_t             mPos;

private:
  size_t index() const
  {
    if constexpr (Dim == 0) {
      return mPos;
    }
    else {
      return mPos == storage().size() ? mPos : mTree.mCache[Dim - 1][mPos];
    }
  }

public:
  Iterator(const DataTree<T>& tree, size_t index)
      : mTree(tree)
  {
    if constexpr (Dim == 0) {
      mPos = index;
    }
    else {
      const auto& cache = mTree.mCache[Dim - 1];
      auto        match = std::lower_bound(cache.begin(), cache.end(), index);
      if (match == cache.end()) {
        mPos = storage().size();
      }
      else {
        mPos = std::distance(cache.begin(), match);
      }
    }
  }

  const InternalStorageT&    storage() const { return mTree.mValues; }
  const std::vector<DepthT>& depths() const { return mTree.mDepths; }
  const InternalStorageT*    internalPtr() const { return &(mTree.mValues); }

  bool operator==(const Iterator& other) const
  {
    return internalPtr() == other.internalPtr() && index() == other.index();
  }

  template<DepthT D2>
  bool operator==(const Iterator<T, D2>& other)
  {
    return internalPtr() == other.internalPtr() && index() == other.index() &&
           (Dim == D2 || mPos == storage().size());
  }

  bool operator!=(const Iterator& other) const { return !(*this == other); }

  const Iterator& operator++()
  {
    if constexpr (Dim == 0) {
      ++mPos;
    }
    else {
      if (++mPos == mTree.mCache[Dim - 1].size()) {
        mPos = storage().size();
      }
    }
    if (index() < storage().size() && depths()[index()] > Dim) {
      mPos = storage().size();
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
    for (size_t i = 0; i < offset && result.mPos < storage().size(); i++) {
      ++result;
    }
    return result;
  }

  Iterator operator+=(size_t offset) { *this = *this + offset; }

  DereferenceT operator*()
  {
    if constexpr (Dim == 0) {
      return storage()[index()];
    }
    else {
      return ReadView<T, Dim>(*this);
    }
  }
};

template<typename T, DepthT Dim>
struct WriteViewBase
{
  static_assert(Dim > 0, "Use references directly for zero dimensional data.");

protected:
  DataTree<T>& mTree;

  WriteViewBase(DataTree<T>& tree)
      : mTree(tree)
  {
    if (mTree.mAccessFlag < 0) {
      throw std::invalid_argument(
        "Cannot create a write-view because the data-tree has at least one active "
        "read-view.");
    }
    mTree.queueDepth(Dim);
    mTree.mAccessFlag++;
    mTree.mIsCacheValid = false;
  }

  ~WriteViewBase()
  {
    mTree.unqueueDepth(Dim);
    mTree.mAccessFlag--;
  }

public:
  void reserve(size_t n) { mTree.reserve(mTree.size() + n); }
};

template<typename T, DepthT Dim>
struct WriteView : public WriteViewBase<T, Dim>
{
  static_assert(Dim > 1, "Dim == 1 case requires a template specialization");

public:
  WriteView(DataTree<T>& tree)
      : WriteViewBase<T, Dim>(tree) {};

  WriteView(const WriteView&) = delete;
  WriteView(WriteView&&)      = delete;
  const WriteView& operator=(const WriteView&) = delete;
  const WriteView& operator=(WriteView&&) = delete;

  WriteView<T, Dim - 1> child() { return WriteView<T, Dim - 1>(this->mTree); }
};

template<typename T>
struct WriteView<T, 1> : public WriteViewBase<T, 1>
{
private:
  size_t mStart;

public:
  using ValueType  = typename DataTree<T>::value_type;
  using value_type = ValueType;

  WriteView(DataTree<T>& tree)
      : WriteViewBase<T, 1>(tree)
      , mStart(tree.size())
  {}

  void push_back(ValueType val)
  {
    DepthT d = mStart == this->mTree.size() ? 1 : 0;
    this->mTree.push_back(d, std::move(val));
  }
};

}  // namespace data
}  // namespace func
}  // namespace gal

#pragma once

#include <stdint.h>
#include <algorithm>
#include <cstdint>
#include <execution>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
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
class Tree
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

  struct Cache
  {
    std::vector<size_t> mDepthScan;
    std::vector<size_t> mOffsets;
    const Tree<T>&      mTree;

    Cache(const Tree<T>& tree)
        : mTree(tree)
    {}

    void clear()
    {
      mDepthScan.clear();
      mOffsets.clear();
    }

    size_t offset(size_t pos, DepthT depth) const
    {
      if (depth > mTree.depth(pos)) {
        return mTree.size() - pos;
      }
      else {
        return mOffsets[mDepthScan[pos] + depth - 1];
      }
    }
  };

  InternalStorageT    mValues;
  std::vector<DepthT> mDepths;
  std::vector<DepthT> mQueuedDepths;
  mutable Cache       mCache;
  mutable int32_t     mAccessFlag   = 0;
  mutable bool        mIsCacheValid = false;

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
    mCache.mDepthScan.resize(mDepths.size());
    std::transform_exclusive_scan(std::execution::par,
                                  mDepths.begin(),
                                  mDepths.end(),
                                  mCache.mDepthScan.begin(),
                                  size_t(0),
                                  std::plus<size_t> {},
                                  [](DepthT d) { return size_t(d); });
    mCache.mOffsets.resize(mCache.mDepthScan.back() + size_t(mDepths.back()));
    DepthT              dmax = maxDepth();
    std::vector<size_t> pegs(dmax, 0);

    for (size_t i = 0; i < mDepths.size(); i++) {
      size_t dcur = size_t(mDepths[i]);
      if (dcur == 0) {
        continue;
      }
      for (size_t d = 0; d < dcur; d++) {
        if (i > 0) {
          size_t& peg                                 = pegs[d];
          mCache.mOffsets[mCache.mDepthScan[peg] + d] = i - peg;
          peg                                         = i;
        }
        mCache.mOffsets[mCache.mDepthScan[i] + d] = mDepths.size() - i;
      }
    }

    mIsCacheValid = true;
  }

public:
  Tree()
      : mCache(*this)
  {}

  DepthT maxDepth() const
  {
    if (mDepths.empty()) {
      return 0;
    }
    else {
      return mDepths[0];
    }
  };

  void resize(size_t n)
  {
    mDepths.resize(n);
    mValues.resize(n);
  }

  DepthT&       depth(size_t i) { return mDepths[i]; }
  const DepthT& depth(size_t i) const { return mDepths[i]; }

  ValueType&       value(size_t i) { return mValues[i]; }
  const ValueType& value(size_t i) const { return mValues[i]; }

  InternalStorageT&          values() { return mValues; }
  const InternalStorageT&    values() const { return mValues; }
  std::vector<DepthT>&       depths() { return mDepths; }
  const std::vector<DepthT>& depths() const { return mDepths; }

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

  friend std::ostream& operator<<(std::ostream& os, const Tree<T>& tree)
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
  using Type = const typename Tree<T>::ValueType&;
};

template<typename T, DepthT Dim>
struct ReadView
{
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

  static constexpr DepthT NDimensions = Dim;

private:
  const Tree<T>& mTree;
  size_t         mIndex;

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

public:
  ReadView(const Tree<T>& src)
      : mTree(src)
      , mIndex(0)
  {
    setReadMode();
  }

  ReadView(Iterator<T, Dim>& iter)
      : mTree(iter.mTree)
      , mIndex(iter.mIndex)
  {
    setReadMode();
  }

  ~ReadView() { mTree.mAccessFlag++; }

  ReadView(const ReadView&) = delete;
  ReadView(ReadView&&)      = delete;
  const ReadView& operator=(const ReadView&) = delete;
  const ReadView& operator=(ReadView&&) = delete;

  const typename Tree<T>::InternalStorageT& storage() const { return mTree.mValues; }

  Iterator<T, Dim - 1> end() const
  {
    return Iterator<T, Dim - 1>(mTree, storage().size());
  }

  Iterator<T, Dim - 1> begin() const { return Iterator<T, Dim - 1>(mTree, mIndex); }

  typename Iterator<T, Dim - 1>::DereferenceT operator[](size_t i)
  {
    return *(begin() + i);
  }

  size_t advanceIndex() const { return mIndex + mTree.mCache.offset(mIndex, Dim); }

  bool canAdvance() const { return advanceIndex() < mTree.size(); }

  bool tryAdvance()
  {
    if (canAdvance()) {
      mIndex = advanceIndex();
      return true;
    }
    return false;
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
  using InternalStorageT = typename Tree<T>::InternalStorageT;

  const Tree<T>& mTree;
  size_t         mIndex;

public:
  Iterator(const Tree<T>& tree, size_t index)
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
      mIndex += mTree.mCache.offset(mIndex, Dim);
    }
    if (mIndex < storage().size() && depths()[mIndex] > Dim) {
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
      return ReadView<T, Dim>(*this);
    }
  }
};

template<typename T, DepthT Dim>
struct WriteViewBase
{
  static_assert(Dim > 0, "Use references directly for zero dimensional data.");

  static constexpr DepthT NDimensions = Dim;

protected:
  Tree<T>& mTree;

  WriteViewBase(Tree<T>& tree)
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
  WriteView(Tree<T>& tree)
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
  using ValueType  = typename Tree<T>::value_type;
  using value_type = ValueType;

  WriteView(Tree<T>& tree)
      : WriteViewBase<T, 1>(tree)
      , mStart(tree.size())
  {}

  void push_back(ValueType val)
  {
    DepthT d = mStart == this->mTree.size() ? 1 : 0;
    this->mTree.push_back(d, std::move(val));
  }
};

template<typename T>
struct IsTreeTuple : public std::false_type
{
};

template<typename TreeT, typename... TreeTs>
struct IsTreeTuple<std::tuple<TreeT, TreeTs...>>
{
  static constexpr bool value =
    IsInstance<Tree, TreeT>::value && IsTreeTuple<std::tuple<TreeTs...>>::value;
};

template<typename TreeT>
struct IsTreeTuple<std::tuple<TreeT>>
{
  static constexpr bool value = IsInstance<Tree, TreeT>::value;
};

template<typename T>
struct IsReadView : public std::false_type
{
};

template<typename T, DepthT Dim>
struct IsReadView<ReadView<T, Dim>> : public std::true_type
{
};

template<typename T>
struct IsWriteView : public std::false_type
{
};

template<typename T, DepthT Dim>
struct IsWriteView<WriteView<T, Dim>> : public std::true_type
{
};

template<typename TreeTupleT, size_t N, typename TArg, typename... TArgs>
struct TreeTupHelper
{
  static constexpr bool ArgIsRead  = IsReadView<TArg>::value;
  static constexpr bool ArgIsWrite = IsWriteView<TArg>::value;
  static constexpr bool IsInput    = std::is_const_v<TArg> || ArgIsRead;

  static constexpr DepthT ArgDepth = (ArgIsRead || ArgIsWrite) ? TArg::NDimensions : 0;

  static void test(const TreeTupleT& trees)
  {
    int32_t treeDepth = int32_t(std::get<N>(trees).maxDepth());
    if constexpr (IsInput) {
      int32_t offset = treeDepth - int32_t(ArgDepth);
    }
    else {
      int32_t offset = int32_t(ArgDepth);
    }
  }
};

template<typename TreeTupleT, typename... TArgs>
void combinations(const TreeTupleT& trees, std::tuple<TArgs*...>& argPtrs)
{
  static_assert(
    IsInstance<std::tuple, TreeTupleT>::value && IsTreeTuple<TreeTupleT>::value,
    "Expecting a tuple of datatrees");

  static constexpr size_t NArgs = sizeof...(TArgs);

  std::array<int32_t, NArgs> offsets;

  // Find max offset.
  // for each arg, create a helper view of depth arg-depth + offset.
  // Depth first traverse all views while making sure you stay in each level for as long
  // as at least one view can be advanced in that level. Use recursion for the depth first
  // traversal. at level corresponding to 0 offset, call the function on the views or
  // references.

  // Incomplete.
  throw std::logic_error("Not Implemented");
}

}  // namespace data
}  // namespace func
}  // namespace gal

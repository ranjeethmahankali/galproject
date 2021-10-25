#pragma once

#include <stdint.h>
#include <algorithm>
#include <execution>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <tbb/tbb.h>

#include <galcore/Traits.h>
#include <galcore/Types.h>

namespace gal {
namespace func {
namespace data {

using DepthT                      = uint8_t;  // Max is 255.
static constexpr DepthT DEPTH_MAX = UINT8_MAX;

/**
 * @brief Datastructure to store arbitrary dimensional trees of a datatype. The tree uses
 * contiguous storage to store the leaf elements. The nested vectors are divided up a bit
 * like the graduations on a measuring tape, where the finest graduations represent leaf
 * nodes, the coarser graduations represent the parent nodes, and so on.
 *
 * @tparam T The type of data stored in the leaf nodes of the tree.
 */
template<typename T>
class Tree
{
public:
  using Type       = T;
  using ValueType  = SafeInstanceType<T>;
  using value_type = ValueType;  // To support stl helper functions.

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
      else if (depth == 0) {
        return 1;
      }
      else if (pos >= mDepthScan.size()) {
        throw std::out_of_range("Index out of range");
      }
      else {
        return mOffsets[mDepthScan[pos] + depth - 1];
      }
    }
  };

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
    ensureDepth(d);
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

public:
  Tree()
      : mCache(*this)
  {}

  const Cache& cache() const { return mCache; }

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
    if (n == 0) {
      return;
    }
    mDepths.resize(n, 0);
    size_t i = mValues.size();
    mValues.resize(n);

    if (!mQueuedDepths.empty()) {
      mDepths[i] = mQueuedDepths.back();
      mQueuedDepths.clear();
    }
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

  /**
   * @brief Pushes an element into the tree.
   *
   * @param d The depth of the new element. The higher the depth, the coarser the
   * graduation in the measuring tape analogy.
   * @param item
   */
  void push_back(DepthT d, T item)
  {
    mValues.emplace_back(std::move(item));
    pushDepth(d);
  }

  void emplace_back(DepthT d)
  {
    mValues.emplace_back();
    pushDepth(d);
  }

  template<typename... TArgs>
  void emplace_back(DepthT d, TArgs... args)
  {
    mValues.emplace_back(args...);
    pushDepth(d);
  }

  void clear()
  {
    mValues.clear();
    mDepths.clear();
    mQueuedDepths.clear();
    mCache.clear();
    mIsCacheValid = false;
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
  // Dereferncing an iterator of an arbitrary dimension should give you a view.
  using Type = ReadView<T, Dim>;
};

template<typename T>
struct Dereferenced<T, 0>
{
  // Dereferencing an iterator of 0 dimensions should return a reference to actual data
  // stored in the tree.
  using Type = const typename Tree<T>::ValueType&;
};

/**
 * @brief A read-only view into the tree, pointing to a single node.
 *
 * @tparam T The type of the data stored in the tree.
 * @tparam Dim The dimensionality of the data in this view, i.e. the height of the node
 * that is being accessed w.r.t. the leaf nodes.
 */
template<typename T, DepthT Dim>
struct ReadView
{
  static_assert(Dim > 0, "Use the reference directly for 0 dimensional views");

  static constexpr DepthT NDimensions = Dim;

private:
  const Tree<T>* mTree;
  size_t         mIndex;

  void setReadMode()
  {
    if (!mTree) {
      return;
    }
    if (mTree->mAccessFlag > 0) {
      throw std::invalid_argument(
        "Cannot create a read-view because the data-tree has at least one active "
        "write-view");
    }
    mTree->ensureCache();
    mTree->mAccessFlag--;
  }

  void releaseReadMode()
  {
    if (mTree) {
      mTree->mAccessFlag++;
    }
  }

public:
  ReadView(const Tree<T>& src)
      : mTree(&src)
      , mIndex(0)
  {
    setReadMode();
  }

  ReadView(const Tree<T>& src, size_t index)
      : mTree(&src)
      , mIndex(index)
  {
    setReadMode();
  }

  ReadView(Iterator<T, Dim>& iter)
      : mTree(&(iter.mTree))
      , mIndex(iter.mIndex)
  {
    setReadMode();
  }

  ReadView(const ReadView& other)
      : mTree(other.mTree)
      , mIndex(other.mIndex)
  {
    setReadMode();
  }

  ReadView(ReadView&& other)
      : mTree(other.mTree)
      , mIndex(other.mIndex)
  {
    other.mTree = nullptr;
  }

  ~ReadView() { releaseReadMode(); }

  const ReadView& operator=(const ReadView& other)
  {
    releaseReadMode();
    mTree  = other.mTree;
    mIndex = other.mIndex;
    setReadMode();
  }
  const ReadView& operator=(ReadView&& other)
  {
    releaseReadMode();
    mTree       = other.mTree;
    mIndex      = other.mIndex;
    other.mTree = nullptr;
  }

  const typename Tree<T>::InternalStorageT& storage() const { return mTree->mValues; }

  Iterator<T, Dim - 1> end() const
  {
    return Iterator<T, Dim - 1>(*mTree, storage().size());
  }

  Iterator<T, Dim - 1> begin() const { return Iterator<T, Dim - 1>(*mTree, mIndex); }

  typename Iterator<T, Dim - 1>::DereferenceT operator[](size_t i)
  {
    return *(begin() + i);
  }

  size_t size() const
  {
    if constexpr (Dim > 0) {
      size_t n = 0;
      size_t i = mIndex;
      do {
        i += mTree->mCache.offset(i, Dim - 1);
        n++;
      } while (i < mTree->size() && mTree->depth(i) == Dim - 1);
      return n;
    }
    else {
      return mTree->mCache.offset(mIndex, Dim);
    }
  }

  const T* data() const { return mTree->mValues.data() + mIndex; }

  /**
   * @brief Index at which the sibling of this node begins.
   *
   * @return size_t
   */
  size_t advanceIndex() const { return mIndex + mTree->mCache.offset(mIndex, Dim); }

  /**
   * @brief Checks whether this tree can advance into its sibling's position. If this node
   * is the last child of its parent (hence no siblings after it), it returns false.
   *
   * @return bool
   */
  bool canAdvance() const { return advanceIndex() < mTree->size(); }

  /**
   * @brief Advances this tree to its sibling's position if possible. If this node is the
   * last child of its parent (canAdvance() returns false), nothing will happen.
   *
   * @return bool True if the tree was advanced, false otherwise.
   */
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

/**
 * @brief Iterator that points to a node in the tree.
 *
 * @tparam T The type of the data in the tree.
 * @tparam Dim The dimensionality of the node.
 */
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

/**
 * @brief Base class for a write-only view into a single node of the tree.
 *
 * @tparam T The type of data in the tree.
 * @tparam Dim The dimensionality of the node.
 */
template<typename T, DepthT Dim>
struct WriteViewBase
{
  static_assert(Dim > 0, "Use references directly for zero dimensional data.");

  static constexpr DepthT NDimensions = Dim;

protected:
  Tree<T>* mTree;

  void setWriteMode()
  {
    if (!mTree) {
      return;
    }
    if (mTree->mAccessFlag < 0) {
      throw std::invalid_argument(
        "Cannot create a write-view because the data-tree has at least one active "
        "read-view.");
    }
    mTree->queueDepth(Dim);
    mTree->mAccessFlag++;
    mTree->mIsCacheValid = false;
  }

  void releaseWriteMode()
  {
    if (mTree) {
      mTree->unqueueDepth(Dim);
      mTree->mAccessFlag--;
    }
  }

  WriteViewBase(Tree<T>* tree)
      : mTree(tree)
  {}

public:
  void reserve(size_t n) { mTree->reserve(mTree->size() + n); }
};

/**
 * @brief A write-only view into a node of the tree.
 *
 * @tparam T The type of the data in the tree.
 * @tparam Dim The dimensionality of the node.
 */
template<typename T, DepthT Dim>
struct WriteView : public WriteViewBase<T, Dim>
{
  static_assert(Dim > 1, "Dim == 1 case requires a template specialization");
  using BaseT = WriteViewBase<T, Dim>;

public:
  WriteView(Tree<T>* treeptr)
      : BaseT(treeptr)
  {
    this->setWriteMode();
  }

  WriteView(Tree<T>& tree)
      : BaseT(&tree)
  {
    this->setWriteMode();
  };

  WriteView(const WriteView& other)
      : BaseT(other.mTree)
  {
    this->setWriteMode();
  }

  WriteView(WriteView&& other)
      : BaseT(other.mTree)
  {
    other.mTree = nullptr;
  }

  ~WriteView() { this->releaseWriteMode(); }

  const WriteView& operator=(const WriteView& other)
  {
    this->releaseWriteMode();
    this->mTree = other.mTree;
    this->setWriteMode();
  }

  const WriteView& operator=(WriteView&& other)
  {
    this->mTree = other.mTree;
    other.mTree = nullptr;
  }

  /**
   * @brief Returns a write-only view into the child node of this node.
   *
   */
  WriteView<T, Dim - 1> child() { return WriteView<T, Dim - 1>(this->mTree); }
};

/**
 * @brief Specialization for 1 dimensional write-only views. This is needed because
 * 1-dimensional write-onyl views need to behave like std::vectors.
 *
 * @tparam T Type of data.
 */
template<typename T>
struct WriteView<T, 1> : public WriteViewBase<T, 1>
{
private:
  size_t mStart;

public:
  using ValueType  = typename Tree<T>::value_type;
  using value_type = ValueType;
  using BaseT      = WriteViewBase<T, 1>;

  WriteView(Tree<T>& tree)
      : BaseT(&tree)
      , mStart(tree.size())
  {
    this->setWriteMode();
  }

  WriteView(Tree<T>* treeptr)
      : BaseT(treeptr)
      , mStart(treeptr->size())
  {
    this->setWriteMode();
  }

  WriteView(const WriteView<T, 1>& other)
      : BaseT(other.mTree)
      , mStart(other.mStart)
  {
    this->setWriteMode();
  }

  WriteView(WriteView<T, 1>&& other)
      : BaseT(other.mTree)
      , mStart(other.mStart)
  {
    other.mTree = nullptr;
  }

  ~WriteView() { this->releaseWriteMode(); }

  const WriteView<T, 1>& operator=(const WriteView<T, 1>& other)
  {
    this->releaseWriteMode();
    this->mTree  = other.mTree;
    this->mStart = other.mStart;
    this->setWriteMode();
  }

  const WriteView<T, 1>& operator=(WriteView<T, 1>&& other)
  {
    this->mTree = other.mTree;
    other.mTree = nullptr;
  }

  /**
   * @brief Current size of view, i.e. the number of elements written into this tree-node
   * so far.
   *
   * @return size_t
   */
  size_t size() const { return this->mTree->size() - mStart; }

  void push_back(ValueType val)
  {
    this->mTree->push_back(DepthT(size() == 0 ? 1 : 0), std::move(val));
  }

  void resize(size_t n)
  {
    if (n == 0) {
      return;
    }
    this->mTree->resize(n + this->mTree->size());
  }

  ValueType&       operator[](size_t i) { return this->mTree->value(mStart + i); }
  const ValueType& operator[](size_t i) const { return this->mTree->value(mStart + i); }
};

/**
 * @brief Template to check for tuples of data trees at compile time.
 *
 * @tparam T
 */
template<typename T>
struct IsTreeTuple : public std::false_type
{
};

template<typename TreeT, typename... TreeTs>
struct IsTreeTuple<std::tuple<TreeT, TreeTs...>>
{
  using TreeCleanT = std::remove_const_t<std::remove_reference_t<TreeT>>;
  static constexpr bool value =
    IsInstance<Tree, TreeCleanT>::value && IsTreeTuple<std::tuple<TreeTs...>>::value;
};

template<typename TreeT>
struct IsTreeTuple<std::tuple<TreeT>>
{
  using TreeCleanT            = std::remove_const_t<std::remove_reference_t<TreeT>>;
  static constexpr bool value = IsInstance<Tree, TreeCleanT>::value;
};

/**
 * Templates to check for various view types for the sake of compile time decision making.
 */

template<typename T>
struct IsReadView : public std::false_type
{
};

template<typename T, DepthT Dim>
struct IsReadView<ReadView<T, Dim>> : public std::true_type
{
};

template<typename T, DepthT Dim>
struct IsReadView<const ReadView<T, Dim>> : public std::true_type
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

template<typename T, DepthT Dim>
struct IsWriteView<const WriteView<T, Dim>> : public std::true_type
{
};

/**
 * More helper templates to help with metaprogramming neeeded to get the combinatorics
 * etc. working.
 */

template<typename T>
struct UnwrapView
{
  using Type = T;
};

template<typename T, DepthT Dim>
struct UnwrapView<ReadView<T, Dim>>
{
  using Type = T;
};

template<typename T, DepthT Dim>
struct UnwrapView<WriteView<T, Dim>>
{
  using Type = T;
};

template<typename T, DepthT Dim>
struct UnwrapView<const ReadView<T, Dim>>
{
  using Type = T;
};

template<typename T, DepthT Dim>
struct UnwrapView<const WriteView<T, Dim>>
{
  using Type = T;
};

template<typename T>
struct ViewDimensions
{
  static constexpr DepthT value = 0;
};

template<typename T, DepthT Dim>
struct ViewDimensions<WriteView<T, Dim>>
{
  static constexpr DepthT value = Dim;
};

template<typename T, DepthT Dim>
struct ViewDimensions<ReadView<T, Dim>>
{
  static constexpr DepthT value = Dim;
};

template<typename T, DepthT Dim>
struct ViewDimensions<const ReadView<T, Dim>>
{
  static constexpr DepthT value = Dim;
};

/* Contains everything related to running the functions several times over large
 * datatrees, including tree combinatorics.*/
namespace repeat {

/**
 * @brief A read-only view into a single node of a tree. This is intended neither for
 * reading nor writing data from / into the tree. Its only for traversing the tree during
 * combinatorics.
 *
 * @tparam T
 */
template<typename T>
struct CombiView
{
private:
  const Tree<T>& mTree;
  size_t         mIndex = 0;
  DepthT         mOffset;
  const DepthT   mArgDepth;

public:
  CombiView(const Tree<T>& tree, DepthT offset, DepthT argDepth)
      : mTree(tree)
      , mOffset(offset)
      , mArgDepth(argDepth)
  {}

  size_t index() const { return mIndex; }

  DepthT offset() const { return mOffset; }

  const Tree<T>& tree() const { return mTree; }

  /**
   * @brief Gets the child view of this view, i.e. a view that points to the child node of
   * the node that this view points to.
   *
   */
  CombiView<T> child() const
  {
    if (mOffset == 0) {
      throw std::logic_error("The leaf view cannot have a child view");
    }
    auto c   = CombiView<T>(mTree, mOffset - 1, mArgDepth);
    c.mIndex = mIndex;
    return c;
  }

  /**
   * @brief Tries to advance this view to its sibling's position.
   *
   * @tparam IsInput flag indicating whether this view points to an input arg.
   * @return bool True if the view was advanced, false otherwise. The tree will not be
   * advanced if this node is the last node if its parent.
   */
  template<bool IsInput>
  bool tryAdvance()
  {
    DepthT td = mArgDepth + mOffset;
    if constexpr (IsInput) {
      if (mIndex >= mTree.size()) {
        return false;
      }
      size_t advIdx = mIndex + (td == 0 ? 1 : mTree.cache().offset(mIndex, td));
      mIndex        = advIdx;
      if (advIdx < mTree.size()) {
        return true;
      }
      return false;
    }
    else {
      if (td == 0) {
        mIndex++;
      }
      return false;
    }
  }
};

// Base template - dummy.
template<size_t NInputs, typename T>
struct CombiViewTuple
{
};

/**
 * @brief Helper class for managing simultaneous views into several trees while doing
 * combinatorics.
 *
 * @tparam NInputs The number of input arguments.
 * @tparam TreeTs The data trees corresponding to all arguments.
 */
template<size_t NInputs, typename... TreeTs>
struct CombiViewTuple<NInputs, std::tuple<TreeTs...>>
{
  template<size_t N>
  using ValueType = typename std::tuple_element_t<
    N,
    std::tuple<std::remove_reference_t<std::remove_const_t<TreeTs>>...>>::Type;

  /**
   * Tuple of combi-views, one each into the data trees corresponding to the function
   * arguments.
   */
  using Type = std::tuple<
    CombiView<typename std::remove_reference_t<std::remove_const_t<TreeTs>>::Type>...>;

  /**
   * Tuple of the trees corresponding to the function arguments.
   */
  using TreeTupleT = std::tuple<TreeTs...>;

  static constexpr size_t NTrees = sizeof...(TreeTs);

private:
  template<size_t... Is>
  static void initInternal(TreeTupleT&                       trees,
                           const std::array<DepthT, NTrees>& viewDepths,
                           DepthT                            offset,
                           std::vector<Type>&                dst,
                           std::index_sequence<Is...>)
  {
    dst.clear();
    for (size_t i = 0; i <= offset; i++) {
      // Go as deep as possible - i.e. until zero offset.
      dst.emplace_back(
        CombiView<ValueType<Is>>(std::get<Is>(trees), offset - i, viewDepths[Is])...);
    }
  }

  template<size_t... Is>
  static Type goDeeperInternal(const Type& leaf, std::index_sequence<Is...>)
  {
    return std::make_tuple(std::get<Is>(leaf).child()...);
  }

  template<typename ArgType, size_t N>
  static ArgType getArg(Type& view, const TreeTupleT& trees)
  {
    auto& v = std::get<N>(view);
    if constexpr (N < NInputs) {
      std::get<N>(trees).ensureCache();
    }

    if constexpr (IsInstance<data::Tree, std::remove_reference_t<ArgType>>::value) {
      return std::get<N>(trees);
    }
    else if constexpr (IsReadView<ArgType>::value) {
      return ArgType(v.tree(), v.index());
    }
    else if constexpr (IsWriteView<ArgType>::value) {
      return ArgType(std::get<N>(trees));
    }
    else {
      if constexpr (N < NInputs) {
        return std::get<N>(trees).value(v.index());
      }
      else {
        if (v.index() == std::get<N>(trees).size()) {
          std::get<N>(trees).emplace_back(0);
          return std::get<N>(trees).values().back();
        }
        else {
          return std::get<N>(trees).value(v.index());
        }
      }
    }
  }

  template<typename ArgsTupleT, size_t... Is>
  static ArgsTupleT getArgsInternal(Type&             view,
                                    const TreeTupleT& trees,
                                    std::index_sequence<Is...>)
  {
    return ArgsTupleT(getArg<std::tuple_element_t<Is, ArgsTupleT>, Is>(view, trees)...);
  }

public:
  /**
   * @brief Initializes the combination stack.
   *
   * @param trees Tuple of datatrees.
   * @param viewDepths Depths of the views requesteed by the function (as arguments).
   * @param offset Maximum offset among all trees. Offset of a tree is the difference
   * between the depth of the tree, and the depth of the view requested by the function.
   * @param dst The combination stack. This will be populated with tuples of combi-views.
   */
  static void init(TreeTupleT&                       trees,
                   const std::array<DepthT, NTrees>& viewDepths,
                   DepthT                            offset,
                   std::vector<Type>&                dst)
  {
    initInternal(trees, viewDepths, offset, dst, std::make_index_sequence<NTrees> {});
  }

  /**
   * @brief Tries to advance all the combiviews int the tuple.
   *
   * @tparam N the index of the tuple member to be advanced, used in recursive calls.
   * @param tup The tuple of combiviews.
   * @return bool True if at least one combiview in the tuple was successfully advanced.
   * This means the function has to run at least once more.
   */
  template<size_t N = 0>
  static bool tryAdvance(Type& tup)
  {
    if constexpr (N < NTrees) {
      static constexpr bool IsInput = N < NInputs;
      // Try to advance the tuple of views. Its successful if at least one view can be
      // advanced. Return true if successful, false otherwise.
      if constexpr (N + 1 == NTrees) {
        return std::get<N>(tup).template tryAdvance<IsInput>();
      }
      else {
        // Try advancing current and next separately to avoid short circuiting the OR.
        bool current = std::get<N>(tup).template tryAdvance<IsInput>();
        bool next    = tryAdvance<N + 1>(tup);
        return current || next;
      }
    }
    else {
      return false;
    }
  }

  /**
   * @brief For the last element of the given combination stack, appends another tuple of
   * combiviews, where each tuple member is the child view of the corresponding member the
   * tuple before.
   *
   * @param dst combination stack.
   */
  static void goDeeper(std::vector<Type>& dst)
  {
    // Push_back another combiview tuple to dst, where each combiview has depth 1 less
    // than the corresponding view in the last elemtn of dst.
    if (dst.empty()) {
      return;
    }
    goDeeperInternal(dst.back(), std::make_index_sequence<NTrees> {});
  }

  /**
   * @brief Converts the tuple of combiviews into a tuple of arguments for the function.
   *
   * @tparam ArgsTupleT Tuple of arguments type.
   * @param view The current view.
   * @param trees The data trees.
   * @return ArgsTupleT tuple of arguments.
   */
  template<typename ArgsTupleT>
  static ArgsTupleT getArgs(Type& view, const TreeTupleT& trees)
  {
    return getArgsInternal<ArgsTupleT>(view, trees, std::make_index_sequence<NTrees> {});
  }
};

/**
 * @brief Represents (conceptually) the collection of all combinations of input arguments
 * for a given set of data-trees and a function that needs to run using those datatrees.
 *
 * @tparam NInputs Number of input arguments.
 * @tparam TreeTupleT Tuple of data trees corresponding to all arguments of the function.
 * @tparam TArgs The types of all arguments of the function.
 */
template<size_t NInputs, typename TreeTupleT, typename... TArgs>
struct Combinations
{
  static constexpr size_t NArgs = sizeof...(TArgs);
  static_assert(IsInstance<std::tuple, TreeTupleT>::value &&
                  IsTreeTuple<TreeTupleT>::value,
                "Expecting a tuple of datatrees");

private:
  template<size_t N = 0>
  static inline void getDepthData(const TreeTupleT&          trees,
                                  std::array<DepthT, NArgs>& viewDepths,
                                  std::array<DepthT, NArgs>& offsets)
  {
    if constexpr (N < NArgs) {
      using TArg                      = std::tuple_element_t<N, std::tuple<TArgs...>>;
      static constexpr bool ArgIsTree = IsInstance<Tree, TArg>::value;
      static constexpr bool ArgIsRead = IsReadView<std::remove_reference_t<TArg>>::value;
      static constexpr bool ArgIsWrite =
        IsWriteView<std::remove_reference_t<TArg>>::value;

      auto treeDepth = std::get<N>(trees).maxDepth();
      if constexpr (ArgIsTree) {
        // The function is requesting the whole tree at once, so the view depth must be
        // the same as the depth of the tree. Hence the offset is 0.
        viewDepths[N] = treeDepth;
        offsets[N]    = 0;
      }
      else {
        static constexpr DepthT ArgDepth =
          (ArgIsRead || ArgIsWrite) ? ViewDimensions<TArg>::value : 0;
        viewDepths[N] = ArgDepth;
        // Prevent unsigned integer underflow.
        offsets[N] = treeDepth < ArgDepth ? 0 : treeDepth - ArgDepth;
      }
    }
    if constexpr (N + 1 < NArgs) {
      getDepthData<N + 1>(trees, viewDepths, offsets);
    }
  }

  using HelperT   = CombiViewTuple<NInputs, TreeTupleT>;
  using CVTupType = typename HelperT::Type;

  TreeTupleT                mTrees;
  std::array<DepthT, NArgs> mViewDepths;
  std::vector<CVTupType>    mViews;
  DepthT                    mMaxOffset = 0;

public:
  /**
   * @brief Create a new set of combinations for the trees. A reference to the datatrees
   * is stored inside this object, so a single instance of this should be created and
   * owned by a function.
   *
   * @param trees Data trees corresponding to the arguments of the function.
   */
  Combinations(const TreeTupleT& trees)
      : mTrees(trees)
  {
    std::array<DepthT, NArgs> offsets;
    getDepthData(trees, mViewDepths, offsets);
    mMaxOffset = *(std::max_element(offsets.begin(), offsets.end()));
    mViews.reserve(size_t(mMaxOffset) + 1);
  }

  /**
   * @brief Initializes a new set of combinations for a new run of the function.
   *
   */
  void init() { HelperT::init(mTrees, mViewDepths, mMaxOffset, mViews); }

  bool empty() const { return mViews.empty(); }

  /**
   * @brief Checks if another combination of inputs is available. If available, the
   * current combination is updated.
   *
   * @return bool True if another combination is available, false otherwise.
   */
  bool next()
  {
    // Try advance the stack.
    while (!mViews.empty() && !HelperT::tryAdvance(mViews.back())) {
      mViews.pop_back();
    }
    if (empty()) {
      return false;
    }
    // Make sure we're at the leaf again.
    while (mViews.size() < mMaxOffset + 1) {
      HelperT::goDeeper(mViews);
    }
    return true;
  }

  /**
   * @brief Gets a tuple of arguments corresponding to the current combination of inputs.
   *
   * @tparam ArgTupleT Argument tuple type.
   * @return ArgTupleT
   */
  template<typename ArgTupleT>
  ArgTupleT current()
  {
    // Get the current top of the stack and create a argument tuple out of it.
    if (mViews.empty()) {
      throw std::logic_error("No combinations letf.");
    }
    return HelperT::template getArgs<ArgTupleT>(mViews.back(), mTrees);
  }
};

}  // namespace repeat

}  // namespace data
}  // namespace func

/**
 * Make data-trees and views compatible with managed types by specializing TypeInfo.
 */

template<typename T>
struct TypeInfo<func::data::Tree<T>> : public TypeInfo<T>
{
};

template<typename T, func::data::DepthT Dim>
struct TypeInfo<func::data::WriteView<T, Dim>> : public TypeInfo<T>
{
};

template<typename T, func::data::DepthT Dim>
struct TypeInfo<func::data::ReadView<T, Dim>> : public TypeInfo<T>
{
};

}  // namespace gal

namespace std {

// This lets STL know how to use the iterator with STL containers and functions.
template<typename T>
struct iterator_traits<gal::func::data::Iterator<T, 0>>
{
  using iterator_type     = T;
  using iterator_category = std::input_iterator_tag;
  using value_type        = T;
  using pointer           = T*;
  using reference         = T&;
};

}  // namespace std

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include <galcore/Util.h>
#include <galfunc/Property.h>

namespace gal {
namespace func {

struct Function;  // Forward decl.

namespace graph {

struct Node
{
  int input  = -1;
  int output = -1;
};

struct Pin
{
  int node = -1;
  int link = -1;
  int prev = -1;
  int next = -1;
};

struct Link
{
  int start = -1;
  int end   = -1;
  int prev  = -1;
  int next  = -1;
};

struct Graph;

template<typename ElemT>
struct IteratorTraverse
{
  static int increment(const Graph& g, int idx);
  static int decrement(const Graph& g, int idx);
};

template<typename ElemT>
struct Iterator
{
private:
  using Traverse = IteratorTraverse<ElemT>;

  const Graph& mGraph;
  int          mIndex = -1;

public:
  explicit Iterator(const Graph& graph, int i = -1)
      : mGraph(graph)
      , mIndex(i)
  {}

  int  operator*() const { return mIndex; }
  bool operator==(const Iterator& other) const
  {
    return &(other.mGraph) == &mGraph && other.mIndex == mIndex;
  }
  bool      operator!=(const Iterator& other) const { return !(*this == other); }
  void      increment() { mIndex = Traverse::increment(mGraph, mIndex); }
  Iterator& operator++()
  {
    increment();
    return *this;
  }
  Iterator operator++(int) const
  {
    Iterator tmp(mGraph, mIndex);
    increment();
    return tmp;
  }
  void      decrement() { mIndex = Traverse::decrement(mGraph, mIndex); }
  Iterator& operator--()
  {
    decrement();
    return *this;
  }
  Iterator operator--(int) const
  {
    Iterator tmp(mGraph, mIndex);
    decrement();
    return tmp;
  }
};

using PinIterator  = Iterator<Pin>;
using LinkIterator = Iterator<Link>;

class Graph
{
public:
  Graph();

  const Node& node(int idx) const;
  Node&       node(int idx);

  const Pin& pin(int idx) const;
  Pin&       pin(int idx);

  const Link& link(int idx) const;
  Link&       link(int idx);

  int nodeInput(int ni) const;
  int nodeInput(int ni, int ii) const;
  int nodeOutput(int ni) const;
  int nodeOutput(int ni, int oi) const;

  int pinNode(int pi) const;
  int pinLink(int pi) const;
  int pinPrev(int pi) const;
  int pinNext(int pi) const;

  int linkStart(int li) const;
  int linkEnd(int li) const;
  int linkPrev(int li) const;
  int linkNext(int li) const;

  PinIterator  nodeInputIter(int ni) const;
  PinIterator  nodeOutputIter(int ni) const;
  LinkIterator pinLinkIter(int pi) const;

  utils::IterSpan<PinIterator>  nodeInputs(int ni) const;
  utils::IterSpan<PinIterator>  nodeOutputs(int ni) const;
  utils::IterSpan<LinkIterator> pinLinks(int pi) const;

  const std::vector<Node>& nodes() const;

  int newNode();
  int newPin();
  int newLink();

  int addNode(size_t nInputs, size_t nOutputs);
  int addLink(int start, int end);

  void setNodeInput(int ni, int i);
  void setNodeOutput(int ni, int i);
  void setPinNode(int pi, int i);
  void setPinLink(int pi, int i);
  void setPinPrev(int pi, int i);
  void setPinNext(int pi, int i);
  void setLinkStart(int li, int i);
  void setLinkEnd(int li, int i);
  void setLinkPrev(int li, int i);
  void setLinkNext(int li, int i);

  size_t numNodes() const;
  size_t numPins() const;
  size_t numLinks() const;

  void clear();
  void reserve(size_t nNodes, size_t nPins, size_t nLinks);

  bool nodeHasOutputs(int ni) const;
  bool nodeHasInputs(int ni) const;

private:
  std::vector<Pin>  mPins;
  std::vector<Link> mLinks;
  std::vector<Node> mNodes;

  Properties mPinPropContainer;
  Properties mLinkPropContainer;
  Properties mNodePropContainer;

public:
  template<typename T>
  Property<T> addNodeProperty()
  {
    auto p = Property<T>(mNodePropContainer);
    mNodePropContainer.resize(mNodes.size());
    return p;
  }

  template<typename T>
  Property<T> addPinProperty()
  {
    auto p = Property<T>(mPinPropContainer);
    mPinPropContainer.resize(mPins.size());
    return p;
  }
};

template<>
struct IteratorTraverse<Pin>
{
  static int increment(const Graph& g, int idx)
  {
    if (idx != -1) {
      return g.pinNext(idx);
    }
    return -1;
  }
  static int decrement(const Graph& g, int idx)
  {
    if (idx != -1) {
      return g.pinPrev(idx);
    }
    return -1;
  }
};

template<>
struct IteratorTraverse<Link>
{
  static int increment(const Graph& g, int idx)
  {
    if (idx != -1) {
      return g.linkNext(idx);
    }
    return -1;
  }
  static int decrement(const Graph& g, int idx)
  {
    if (idx != -1) {
      return g.linkPrev(idx);
    }
    return -1;
  }
};

}  // namespace graph
}  // namespace func
}  // namespace gal

namespace std {

template<typename T>
struct iterator_traits<gal::func::graph::Iterator<T>>
{
  using iterator_type     = int;
  using iterator_category = std::input_iterator_tag;
  using value_type        = int;
  using difference_type   = int;
  using pointer           = int*;
  using reference         = int&;
};

}  // namespace std

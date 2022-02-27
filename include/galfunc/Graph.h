#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>

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

struct BaseIterator
{
  const Graph& mGraph;
  int          mIndex = -1;

  BaseIterator(const Graph& graph, int i = -1);

  int  operator*() const;
  bool valid() const;

  bool operator==(const BaseIterator& other)
  {
    return &(other.mGraph) == &mGraph && other.mIndex == mIndex;
  }

  bool operator!=(const BaseIterator& other) { return *this != other; }
};

struct PinIterator;
struct LinkIterator;

struct NodeProps
{
  int depth  = -1;
  int height = -1;
};

template<typename IterT>
class Range
{
  IterT mBegin;
  IterT mEnd;

public:
  Range(IterT b, IterT e)
      : mBegin(b)
      , mEnd(e)
  {}

  IterT begin() { return mBegin; }

  IterT end() { return mEnd; }
};

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

  Range<PinIterator>  nodeInputs(int ni) const;
  Range<PinIterator>  nodeOutputs(int ni) const;
  Range<LinkIterator> pinLinks(int pi) const;

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

  int nodeDepth(int ni) const;
  int nodeHeight(int ni) const;

  void clear();
  void reserve(size_t nNodes, size_t nPins, size_t nLinks);

  static void build(Graph& g, Property<int>& funcNodeIndices);

private:
  std::vector<Pin>  mPins;
  std::vector<Link> mLinks;
  std::vector<Node> mNodes;

  Properties mPinPropContainer;
  Properties mLinkPropContainer;
  Properties mNodePropContainer;

  Property<NodeProps> mNodeProps;

public:
  template<typename T>
  Property<T> addNodeProperty()
  {
    mNodePropContainer.resize(mNodes.size());
    return Property<T>(mNodePropContainer);
  }
};

struct PinIterator : public BaseIterator
{
  PinIterator(const Graph& g, int i = -1);

  void        increment();
  PinIterator operator++();
  PinIterator operator++(int);

  void        decrement();
  PinIterator operator--();
  PinIterator operator--(int);
};

struct LinkIterator : public BaseIterator
{
  LinkIterator(const Graph& g, int i = -1);

  void         increment();
  LinkIterator operator++();
  LinkIterator operator++(int);

  void         decrement();
  LinkIterator operator--();
  LinkIterator operator--(int);
};

}  // namespace graph
}  // namespace func
}  // namespace gal

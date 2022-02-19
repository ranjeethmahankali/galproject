#pragma once
#include <type_traits>
#include <utility>
#include <vector>

namespace gal {
namespace func {
namespace graph {

struct IPropData
{
  virtual void reserve(size_t n) = 0;
  virtual void resize(size_t n)  = 0;
  virtual void clear()           = 0;
};

template<typename T>
struct PropData : public IPropData, public std::vector<T>
{
};

template<typename T>
struct Property
{
  int index = -1;

public:
  Property()
  {
    // TODO: Add a new propdata to the store and capture index and reference to propdata.
  }
  ~Property()
  {
    // TODO: delete property from the store using the index.
  }
  // Forbid copy.
  Property(const Property&) = delete;
  const Property& operator=(const Property&) = delete;
  // Move semantics.
  const Property& operator=(Property&& other)
  {
    if (index == other.index && this == &other) {
      return *this;
    }
    if (index != -1) {
      // TODO: delete this property from the store using the index.
    }
    index = std::exchange(other.index, -1);
    return *this;
  }
  Property(Property&& other) { *this = other; }
};

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
};

struct PinIterator;
struct LinkIterator;

class Graph
{
public:
  const Node& node(int idx) const;
  Node&       node(int idx);

  const Pin& pin(int idx) const;
  Pin&       pin(int idx);

  const Link& link(int idx) const;
  Link&       link(int idx);

  int nodeInput(int ni) const;
  int nodeOutput(int ni) const;

  int pinNode(int pi) const;
  int pinLink(int pi) const;
  int pinPrev(int pi) const;
  int pinNext(int pi) const;

  int linkStart(int li) const;
  int linkEnd(int li) const;
  int linkPrev(int li) const;
  int linkNext(int li) const;

  // Iterators and ranges.
  PinIterator nodeInputIter(int ni) const;
  PinIterator nodeOutputIter(int ni) const;

  LinkIterator pinLinkIter(int pi) const;

private:
  std::vector<Pin>  mPins;
  std::vector<Link> mLinks;
  std::vector<Node> mNodes;

  std::vector<IPropData> mPinProps;
  std::vector<IPropData> mLinkProps;
  std::vector<IPropData> mNodeProps;
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

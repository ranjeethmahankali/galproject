#include <galfunc/Graph.h>

namespace gal {
namespace func {
namespace graph {

BaseIterator::BaseIterator(const Graph& graph, int i)
    : mGraph(graph)
    , mIndex(i)
{}

int BaseIterator::operator*() const
{
  return mIndex;
}

bool BaseIterator::valid() const
{
  return mIndex != -1;
}

const Node& Graph::node(int i) const
{
  return mNodes[i];
}

Node& Graph::node(int i)
{
  return mNodes[i];
}

const Pin& Graph::pin(int i) const
{
  return mPins[i];
}

Pin& Graph::pin(int i)
{
  return mPins[i];
}

const Link& Graph::link(int i) const
{
  return mLinks[i];
}

Link& Graph::link(int i)
{
  return mLinks[i];
}

int Graph::nodeInput(int ni) const
{
  return node(ni).input;
}

int Graph::nodeOutput(int ni) const
{
  return node(ni).output;
}

int Graph::pinNode(int pi) const
{
  return pin(pi).node;
}

int Graph::pinLink(int pi) const
{
  return pin(pi).link;
}

int Graph::pinPrev(int pi) const
{
  return pin(pi).prev;
}

int Graph::pinNext(int pi) const
{
  return pin(pi).next;
}

int Graph::linkStart(int li) const
{
  return link(li).start;
}

int Graph::linkEnd(int li) const
{
  return link(li).end;
}

int Graph::linkPrev(int li) const
{
  return link(li).prev;
}

int Graph::linkNext(int li) const
{
  return link(li).next;
}

PinIterator Graph::nodeInputIter(int ni) const
{
  return PinIterator(*this, node(ni).input);
}

PinIterator Graph::nodeOutputIter(int ni) const
{
  return PinIterator(*this, node(ni).output);
}

LinkIterator Graph::pinLinkIter(int pi) const
{
  return LinkIterator(*this, pin(pi).link);
}

Range<PinIterator> Graph::nodeInputs(int ni) const
{
  return Range<PinIterator>(nodeInputIter(ni), PinIterator(*this));
}

Range<PinIterator> Graph::nodeOutputs(int ni) const
{
  return Range<PinIterator>(nodeOutputIter(ni), PinIterator(*this));
}

Range<LinkIterator> Graph::pinLinks(int pi) const
{
  return Range<LinkIterator>(pinLinkIter(pi), LinkIterator(*this));
}

int Graph::newNode()
{
  int ni = int(mNodes.size());
  mNodes.emplace_back();
  return ni;
}

int Graph::newPin()
{
  int pi = int(mPins.size());
  mPins.emplace_back();
  return pi;
}

int Graph::newLink()
{
  int li = int(mLinks.size());
  mLinks.emplace_back();
  return li;
}

void Graph::setNodeInput(int ni, int i)
{
  node(ni).input = i;
  setPinNode(i, ni);
}

void Graph::setNodeOutput(int ni, int i)
{
  node(ni).output = i;
  setPinNode(i, ni);
}

void Graph::setPinNode(int pi, int i)
{
  pin(pi).node = i;
}

void Graph::setPinLink(int pi, int i)
{
  pin(pi).link = i;
}

void Graph::setPinPrev(int pi, int i)
{
  pin(pi).prev = i;
  pin(i).next  = pi;
}

void Graph::setPinNext(int pi, int i)
{
  pin(pi).next = i;
  pin(i).prev  = i;
}

void Graph::setLinkStart(int li, int i)
{
  link(li).start = i;
  setPinLink(i, li);
}

void Graph::setLinkEnd(int li, int i)
{
  link(li).end = i;
  setPinLink(i, li);
}

void Graph::setLinkPrev(int li, int i)
{
  link(li).prev = i;
  link(i).next  = li;
}

void Graph::setLinkNext(int li, int i)
{
  link(li).next = i;
  link(i).prev  = li;
}

PinIterator::PinIterator(const Graph& g, int i /* = -1*/)
    : BaseIterator(g, i)
{}

void PinIterator::increment()
{
  if (valid()) {
    mIndex = mGraph.pinNext(mIndex);
  }
}

PinIterator PinIterator::operator++()
{
  increment();
  return *this;
}

PinIterator PinIterator::operator++(int)
{
  PinIterator tmp(mGraph, mIndex);
  increment();
  return tmp;
}

void PinIterator::decrement()
{
  if (valid()) {
    mIndex = mGraph.pinPrev(mIndex);
  }
}

PinIterator PinIterator::operator--()
{
  decrement();
  return *this;
}

PinIterator PinIterator::operator--(int)
{
  PinIterator tmp(mGraph, mIndex);
  decrement();
  return tmp;
}

LinkIterator::LinkIterator(const Graph& g, int i /* = -1*/)
    : BaseIterator(g, i)
{}

void LinkIterator::increment()
{
  if (valid()) {
    mIndex = mGraph.linkNext(mIndex);
  }
}

LinkIterator LinkIterator::operator++()
{
  increment();
  return *this;
}

LinkIterator LinkIterator::operator++(int)
{
  LinkIterator temp(mGraph, mIndex);
  increment();
  return temp;
}

void LinkIterator::decrement()
{
  if (valid()) {
    mIndex = mGraph.linkPrev(mIndex);
  }
}

LinkIterator LinkIterator::operator--()
{
  decrement();
  return *this;
}

LinkIterator LinkIterator::operator--(int)
{
  LinkIterator temp(mGraph, mIndex);
  decrement();
  return temp;
}

}  // namespace graph
}  // namespace func
}  // namespace gal

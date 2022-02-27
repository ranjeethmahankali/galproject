#include <galfunc/Functions.h>
#include <galfunc/Graph.h>
#include <cstddef>
#include <numeric>
#include <stdexcept>

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

Graph::Graph()
    : mNodeProps(mNodePropContainer)
{
  mNodePropContainer.resize(mNodes.size());
  mPinPropContainer.resize(mPins.size());
  mLinkPropContainer.resize(mLinks.size());
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

int Graph::nodeInput(int ni, int ii) const
{
  int input = nodeInput(ni);
  for (int i = 0; i < ii && input != -1; i++, input = pinNext(input)) {
    // Do nothing
  }
  return input;
}

int Graph::nodeOutput(int ni) const
{
  return node(ni).output;
}

int Graph::nodeOutput(int ni, int oi) const
{
  int output = nodeOutput(ni);
  for (int i = 0; i < oi && output != -1; i++, output = pinNext(output)) {
    // Do nothing.
  }
  return output;
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

const std::vector<Node>& Graph::nodes() const
{
  return mNodes;
}

int Graph::newNode()
{
  int ni = int(mNodes.size());
  mNodes.emplace_back();
  mNodePropContainer.resize(mNodes.size());
  return ni;
}

int Graph::newPin()
{
  int pi = int(mPins.size());
  mPins.emplace_back();
  mPinPropContainer.resize(mPins.size());
  return pi;
}

int Graph::newLink()
{
  int li = int(mLinks.size());
  mLinks.emplace_back();
  mLinkPropContainer.resize(mLinks.size());
  return li;
}

int Graph::addNode(size_t nInputs, size_t nOutputs)
{
  int ni  = newNode();
  int lpi = -1;
  for (size_t i = 0; i < nInputs; i++) {
    int pi = newPin();
    if (i == 0) {
      setNodeInput(ni, pi);
    }
    else {
      setPinNode(pi, ni);
      setPinPrev(pi, lpi);
    }
    lpi = pi;
  }

  lpi = -1;
  for (size_t i = 0; i < nOutputs; i++) {
    int pi = newPin();
    if (i == 0) {
      setNodeOutput(ni, pi);
      lpi = pi;
    }
    else {
      setPinNode(pi, ni);
      setPinPrev(pi, lpi);
    }
    lpi = pi;
  }
  return ni;
}

int Graph::addLink(int start, int end)
{
  int li = newLink();
  setLinkStart(li, start);
  if (pinLink(start) == -1) {
    setPinLink(start, li);
  }
  setLinkEnd(li, end);
  if (pinLink(end) == -1) {
    setPinLink(end, li);
  }
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
}

void Graph::setLinkEnd(int li, int i)
{
  link(li).end = i;
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

size_t Graph::numNodes() const
{
  return mNodes.size();
}

size_t Graph::numPins() const
{
  return mPins.size();
}

size_t Graph::numLinks() const
{
  return mLinks.size();
}

int Graph::nodeDepth(int ni) const
{
  return mNodeProps[ni].depth;
}

int Graph::nodeHeight(int ni) const
{
  return mNodeProps[ni].height;
}

void Graph::clear()
{
  mPins.clear();
  mLinks.clear();
  mNodes.clear();

  mPinPropContainer.clear();
  mLinkPropContainer.clear();
  mNodePropContainer.clear();
}

void Graph::reserve(size_t nNodes, size_t nPins, size_t nLinks)
{
  mNodes.reserve(nNodes);
  mPins.reserve(nPins);
  mLinks.reserve(nLinks);

  mNodePropContainer.reserve(nNodes);
  mPinPropContainer.reserve(nPins);
  mLinkPropContainer.reserve(nLinks);
}

void Graph::build(Graph&                     g,
                  Property<int>&             funcNodeIndices,
                  Property<const Function*>& nodeFuncs)
{
  g.clear();
  // Reserve memory.
  size_t nfunc    = store::numFunctions();
  size_t nInputs  = 0;
  size_t nOutputs = 0;
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f = store::function(i);
    nInputs += f.numInputs();
    nOutputs = f.numOutputs();
  }
  g.reserve(nfunc, nInputs + nOutputs, nInputs);
  // Add nodes.
  std::vector<InputInfo> inputs;
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f      = store::function(i);
    int         ni     = g.addNode(f.numInputs(), f.numOutputs());
    funcNodeIndices[f] = ni;
    nodeFuncs[ni]      = &f;
  }
  // Add links.
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f = store::function(i);
    f.getInputs(inputs);
    int fni = funcNodeIndices[f];
    int end = g.nodeInput(fni);
    for (const auto& input : inputs) {
      int start = g.nodeOutput(funcNodeIndices[*(input.mFunc)], input.mOutputIdx);
      g.addLink(start, end);
      end = g.pinNext(end);
    }
  }
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

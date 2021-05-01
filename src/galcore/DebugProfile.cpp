#include <galcore/DebugProfile.h>

namespace gal {
namespace debug {

ContextNode  ContextNode::sRoot    = ContextNode("", nullptr);
ContextNode* ContextNode::sCurrent = &ContextNode::sRoot;

ContextNode::ContextNode(const std::string& name, ContextNode* parent)
    : mName(name)
    , mParent(parent)
{
}

ContextNode* ContextNode::addChild(const std::string& name)
{
  mChildren.emplace_back(name, this);
  return &mChildren.back();
}

void ContextNode::push(const std::string& name)
{
  sCurrent = sCurrent->addChild(name);
}

void ContextNode::pop()
{
  sCurrent = sCurrent->mParent;
}

ScopedContext::ScopedContext(const std::string& name)
{
  ContextNode::push(name);
}

ScopedContext::~ScopedContext()
{
  ContextNode::pop();
}

}  // namespace debug
}  // namespace gal
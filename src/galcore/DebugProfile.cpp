#include <galcore/DebugProfile.h>

namespace gal {
namespace debug {

ContextTree  ContextTree::sRoot    = ContextTree("", nullptr);
ContextTree* ContextTree::sCurrent = &ContextTree::sRoot;

ContextTree::ContextTree(const std::string& name, ContextTree* parent)
    : mName(name)
    , mParent(parent)
{
}

ContextTree* ContextTree::addChild(const std::string& name)
{
  mChildren.emplace_back(name, this);
  return &mChildren.back();
}

void ContextTree::push(const std::string& name)
{
  sCurrent = sCurrent->addChild(name);
}

void ContextTree::pop()
{
  sCurrent = sCurrent->mParent;
}

ScopedContext::ScopedContext(const std::string& name)
{
  ContextTree::push(name);
}

ScopedContext::~ScopedContext()
{
  ContextTree::pop();
}

}  // namespace debug
}  // namespace gal
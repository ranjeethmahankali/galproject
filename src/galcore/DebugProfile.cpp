#include <galcore/DebugProfile.h>
#include <galcore/Serialization.h>
#include <galcore/Util.h>
#include <fstream>

namespace gal {
namespace debug {

ContextNode     ContextNode::sRoot    = ContextNode("root", nullptr);
ContextNode*    ContextNode::sCurrent = &ContextNode::sRoot;
static uint64_t sCurrentId            = 0;

fs::path indexFilePath()
{
  return fs::path(sDebugDir) / fs::path(sIndexFile);
}

ContextNode::ContextNode(const std::string& name, ContextNode* parent)
    : mName(name)
    , mParent(parent)
    , mId(sCurrentId++)  // TODO: This is not thread safe!
{
  std::ofstream indexFile;
  if (!mParent) {
    // This means we initialized the root scope.
    std::filesystem::create_directory(sDebugDir);
    indexFile.open(indexFilePath(), std::ios::out | std::ios::trunc);
  }
  else {
    indexFile.open(indexFilePath(), std::ios::out | std::ios::app);
  }

  mDepth = mParent ? mParent->mDepth + 1 : 0;

  if (!indexFile) {
    throw std::filesystem::filesystem_error("Cannot open the index file.",
                                            std::error_code());
  }

  for (uint32_t i = 0; i < mDepth; i++) {
    indexFile << "*";
  }
  indexFile << mName << " " << mId << std::endl;
  indexFile.close();
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
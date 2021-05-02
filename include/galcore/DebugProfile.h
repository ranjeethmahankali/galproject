#pragma once

#include <galcore/Serialization.h>
#include <galcore/Util.h>
#include <filesystem>
#include <string>
#include <vector>

namespace gal {
namespace debug {

static constexpr char sDebugDir[]      = ".galdebug";
static constexpr char sIndexFile[]     = "index";
static constexpr char sCallStackFile[] = "callstack";

namespace fs = std::filesystem;

fs::path indexFilePath();
fs::path callStackPath();

struct ContextNode
{
  ContextNode(const std::string& name, ContextNode* parent);
  static void push(const std::string& name);
  static void pop();

private:
  static ContextNode  sRoot;
  static ContextNode* sCurrent;

  ContextNode* addChild(const std::string& name);

  std::vector<ContextNode> mChildren;
  std::string              mName;
  ContextNode*             mParent;
  uint64_t                 mId;
  uint32_t                 mDepth;

public:
  template<typename T>
  static void capture(const T& var, const std::string& name)
  {
    Bytes data;
    data << var;
    data.saveToFile(utils::absPath(fs::path(sDebugDir) /
                                   fs::path(std::to_string(sCurrent->mId) + "_" + name)));
  };
};

struct ScopedContext
{
public:
  ScopedContext(const std::string& name);
  ~ScopedContext();
};

}  // namespace debug
}  // namespace gal

/**
 * @brief The name of this variable must be fixed. This prevents two scopes from being
 * initialized in the same scope. The random alphanumeric suffix is just there so that
 * it doesn't actually conflict with an actual variable that someone might want to declare
 * in that scope
 */
#ifndef NDEBUG
#define GALSCOPE(name) gal::debug::ScopedContext scope_50e31b17d776(name)
#else
#define GALSCOPE(name)
#endif

#ifndef NDEBUG
#define GALCAPTURE(var) gal::debug::ContextNode::capture(var, #var)
#else
#define GALCAPTURE(var)
#endif
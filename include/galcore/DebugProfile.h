#pragma once

#include <galcore/Serialization.h>
#include <galcore/Types.h>
#include <galcore/Util.h>
#include <filesystem>
#include <string>
#include <vector>

namespace gal {
namespace debug {

static constexpr char sDebugDir[]      = ".galdebug";
static constexpr char sIndexFile[]     = "index";
static constexpr char sCallStackFile[] = "callstack";
static constexpr char sDbgExt[]        = ".galdbg";
static constexpr char sDbgExtTemp[]    = ".galdbgtemp";

namespace fs = std::filesystem;

fs::path indexFilePath();
fs::path callStackPath();
bool     isDebuggingEnabled();
void     enableDebugging();

struct ContextNode
{
  ContextNode(const std::string& name, ContextNode* parent);
  static void push(const std::string& name);
  static void pop();

  static std::shared_ptr<ContextNode> sRoot;
  static ContextNode*                 sCurrent;

private:
  ContextNode* addChild(const std::string& name);

  void deleteCapturedVars();

  std::vector<ContextNode> mChildren;
  std::string              mName;
  ContextNode*             mParent;
  uint64_t                 mId;
  uint32_t                 mDepth;
  std::vector<fs::path>    mCaptured;

  template<typename T>
  void captureInternal(const T& var, const std::string& name)
  {
    Bytes data;
    data << TypeInfo<T>::id;
    data << var;
    fs::path varpath = utils::absPath(
      fs::path(sDebugDir) / fs::path(std::to_string(mId) + "_" + name + sDbgExt));
    mCaptured.push_back(varpath);
    fs::path tempPath = varpath;
    tempPath.replace_extension(sDbgExtTemp);
    data.saveToFile(tempPath);
    fs::rename(tempPath, varpath);
  };

public:
  template<typename T>
  static void capture(const T& var, const std::string& name)
  {
    sCurrent->captureInternal(var, name);
  }
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
#define GALDEBUG
#endif

#ifdef GALDEBUG
#define GALSCOPE(name) gal::debug::ScopedContext scope_50e31b17d776(name)
#define GALCAPTURE(var) gal::debug::ContextNode::capture(var, #var)
#define GALCAPTURE_WITH_NAME(data, name) gal::debug::ContextNode::capture(data, #name)
#define GALCAPTURE_WITH_STRING_NAME(data, name) \
  gal::debug::ContextNode::capture(data, name)

#else

#define GALSCOPE(name)
#define GALCAPTURE(var)
#define GALCAPTURE_WITH_NAME(data, name)
#define GALCAPTURE_WITH_STRING_NAME(data, name)

#endif

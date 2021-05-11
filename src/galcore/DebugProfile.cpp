#include <galcore/DebugProfile.h>
#include <fstream>

namespace gal {
namespace debug {

ContextNode     ContextNode::sRoot    = ContextNode("root", nullptr);
ContextNode*    ContextNode::sCurrent = &ContextNode::sRoot;
static uint64_t sCurrentId            = 0;

fs::path indexFilePath()
{
  return utils::absPath(fs::path(sDebugDir) / fs::path(sIndexFile));
}

fs::path callStackPath()
{
  return utils::absPath(fs::path(sDebugDir) / fs::path(sCallStackFile));
}

static void deleteDirContent(const fs::path& dir_path)
{
  for (auto& path : fs::directory_iterator(dir_path)) {
    fs::remove_all(path);
  }
}

ContextNode::ContextNode(const std::string& name, ContextNode* parent)
    : mName(name)
    , mParent(parent)
    , mId(sCurrentId++)  // TODO: This is not thread safe!
{
  std::ofstream indexFile;
  std::ofstream stackFile;
  if (!mParent) {
    // This means we initialized the root scope.
    std::filesystem::create_directory(sDebugDir);
    deleteDirContent(sDebugDir);

    indexFile.open(indexFilePath(), std::ios::out | std::ios::trunc);
    stackFile.open(callStackPath(), std::ios::out | std::ios::trunc);
    mDepth = 0;
  }
  else {
    indexFile.open(indexFilePath(), std::ios::out | std::ios::app);
    stackFile.open(callStackPath(), std::ios::out | std::ios::app);
    mDepth = mParent->mDepth + 1;
  }

  if (!indexFile) {
    std::cerr << indexFilePath() << std::endl;
    throw std::filesystem::filesystem_error("Cannot open the index file.",
                                            std::error_code());
  }

  for (uint32_t i = 0; i < mDepth; i++) {
    indexFile << "*";
  }
  indexFile << mName << " " << mId << std::endl;
  indexFile.close();

  if (!stackFile) {
    throw std::filesystem::filesystem_error("Cannot open the callstack file.",
                                            std::error_code());
  }

  stackFile << mName << " " << mId << std::endl;
  stackFile.close();
}

ContextNode* ContextNode::addChild(const std::string& name)
{
  mChildren.emplace_back(name, this);
  return &mChildren.back();
}

void ContextNode::deleteCapturedVars()
{
  for (const fs::path& p : mCaptured) {
    fs::remove(p);
  }
  mCaptured.clear();
}

void ContextNode::push(const std::string& name)
{
  sCurrent = sCurrent->addChild(name);
}

static void popStackFile()
{
  static constexpr char tempFile[] = "tempCallStack";

  auto path     = callStackPath();
  auto tempPath = path.parent_path() / fs::path(tempFile);

  std::ifstream fin(path, std::ios::in);
  std::ofstream fout(tempPath, std::ios::out | std::ios::trunc);
  std::string   line;
  std::getline(fin, line);
  for (std::string temp; std::getline(fin, temp); line.swap(temp)) {
    fout << line << std::endl;
  }
  fin.close();
  fout.close();

  fs::rename(tempPath, path);
}

void ContextNode::pop()
{
  sCurrent->deleteCapturedVars();
  sCurrent = sCurrent->mParent;
  popStackFile();
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
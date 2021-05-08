#include <galcore/DebugProfile.h>
#include <galcore/PointCloud.h>
#include <galcore/Serialization.h>
#include <galcore/Types.h>
#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/DebugGeom.h>
#include <galview/Widget.h>
#include <efsw/efsw.hpp>
#include <fstream>

namespace gal {
namespace debug {

class DebugFrame;
class FSListener;

static std::string                              sDebugDirPath;
static std::vector<std::shared_ptr<DebugFrame>> sFrames;
static std::shared_ptr<FSListener>              sFileListener;
static std::shared_ptr<efsw::FileWatcher>       sFileWatcher;
static efsw::WatchID                            sWatchId;

static view::Panel& debugPanel()
{
  static view::Panel& sDebugPanel = view::newPanel("Debug Geometry");
  return sDebugPanel;
}

static view::Panel& outputsPanel()
{
  static view::Panel& sOutputPanel = view::newPanel("Outputs");
  return sOutputPanel;
}

template<typename T, typename... TRest>
struct WatchManager
{
  static_assert(TypeInfo<T>::value, "Not a known type.");

  static void watch(uint32_t typeId, Bytes& bytes, const std::string& geomKey)
  {
    if (typeId == TypeInfo<T>::id) {
      if constexpr (view::MakeDrawable<T>::value) {
        view::Context::get().addDrawable(
          Serial<T>::deserialize(bytes),
          outputsPanel().newWidget<view::CheckBox>(geomKey, true)->checkedPtr());
      }
      else {
        T instance;
        bytes >> instance;
        std::stringstream ss;
        ss << instance;
        outputsPanel().newWidget<view::Text>(geomKey + ": " + ss.str());
      }
    }
    else if constexpr (sizeof...(TRest) > 0) {
      WatchManager<TRest...>::watch(typeId, bytes, geomKey);
    }
    else if constexpr (sizeof...(TRest) == 0) {
      std::cerr << "Datatype " << gal::TypeInfo<T>::name
                << " is not watchable in a debug session\n";
      throw std::bad_cast();
    }
  };
};

using manager = WatchManager<glm::vec2, Circle2d, Box3, Mesh, Sphere, PointCloud>;

class DebugFrame : public gal::view::TextInputBox
{
  std::string mName;
  std::string mPrefix;

public:
  DebugFrame(const std::string& name, uint64_t id)
      : view::TextInputBox(name)
      , mPrefix(std::to_string(id) + "_")
  {}

private:
  using gal::view::TextInputBox::addHandler;

public:
  void reloadChanges()
  {
    view::Context::get().clearDrawables();
    outputsPanel().clearWidgets();
    std::stringstream ss(mValue);
    std::string       name;
    while (std::getline(ss, name)) {
      if (name.empty()) {
        continue;
      }
      fs::path path = sDebugDirPath / fs::path(sDebugDir) / fs::path(mPrefix + name);
      if (fs::exists(path) && !fs::is_directory(path)) {
        Bytes    bytes = Bytes::loadFromFile(path);
        uint32_t typeId;
        bytes >> typeId;
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
        Bytes nested;
        bytes.readNested(nested);
        manager::watch(typeId, nested, name);
        std::cout << "Loading the debug geometry from path: " << path << std::endl;
      }
      else {
        std::cout << "Cannot find the file: " << path << std::endl;
      }
    }
  }

protected:
  void handleChanges() override
  {
    if (!isEdited())
      return;
    reloadChanges();
    clearEdited();
  }
};

static fs::path stackfilepath()
{
  return sDebugDirPath / fs::path(sDebugDir) / fs::path(sCallStackFile);
}

static void reloadAllFrames()
{
  for (auto& frame : sFrames) {
    frame->reloadChanges();
  }
}

class FSListener : public efsw::FileWatchListener
{
public:
  FSListener() = default;

  void handleFileAction(efsw::WatchID      watchid,
                        const std::string& dir,
                        const std::string& filename,
                        efsw::Action       action,
                        std::string        oldFilename = "")
  {
    switch (action) {
    case efsw::Actions::Add:
    case efsw::Actions::Delete:
    case efsw::Actions::Modified:
    case efsw::Actions::Moved:
      reloadAllFrames();
    }
  }
};

void initSession(const fs::path& dirpath)
{
  sDebugDirPath = dirpath;
  if (!fs::exists(stackfilepath())) {
    throw std::runtime_error("Cannot find the stack file");
  }
  loadCallstack();
  outputsPanel();

  // Start watching the folder.
  sFileWatcher  = std::make_shared<efsw::FileWatcher>();
  sFileListener = std::make_shared<FSListener>();
  sWatchId      = sFileWatcher->addWatch(sDebugDir, sFileListener.get(), false);
}

static void pushFrame(const std::string& name, uint64_t id)
{
  sFrames.push_back(debugPanel().newWidget<DebugFrame>(name, id));
}

void loadCallstack()
{
  std::ifstream stackfile(stackfilepath(), std::ios::in);

  std::string       line;
  std::string       word;
  std::stringstream ss;
  while (std::getline(stackfile, line)) {
    ss << line;
    uint64_t id;
    ss >> word >> id;
    pushFrame(word, id);
    ss.clear();
  }
}

}  // namespace debug
}  // namespace gal
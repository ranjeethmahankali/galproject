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

static fs::path stackfiledir()
{
  return sDebugDirPath / fs::path(sDebugDir);
}

static fs::path stackfilepath()
{
  return stackfiledir() / fs::path(sCallStackFile);
}

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

class DebugFrame : public gal::view::Text
{
  std::string mName;
  std::string mPrefix;
  uint64_t    mId;

public:
  DebugFrame(const std::string& name, uint64_t id)
      : view::Text(name)
      , mName(name)
      , mId(id)
      , mPrefix(std::to_string(id) + "_")
  {}

  const std::string& name() const { return mName; }

  uint64_t id() const { return mId; }

  fs::path varfilepath(const std::string& name) const
  {
    return stackfiledir() / fs::path(mPrefix + name);
  }

  bool hasVariable(const std::string& name) const
  {
    return fs::exists(varfilepath(name));
  }
};

class VariablesBox : public gal::view::TextInputBox
{
public:
  VariablesBox()
      : TextInputBox("")
  {}

private:
  using gal::view::TextInputBox::addHandler;

  static std::shared_ptr<VariablesBox> instance()
  {
    static std::shared_ptr<VariablesBox> sInstance =
      debugPanel().newWidget<VariablesBox>();
    return sInstance;
  }

public:
  static void init()
  {
    auto temp = instance();
    if (!temp) {
      std::cerr << "Unable to initialize the variables box\n";
    }
  }
  static void reloadChanges()
  {
    outputsPanel().clearWidgets();
    view::Context::get().clearDrawables();
    std::stringstream ss(instance()->mValue);
    std::string       name;
    while (std::getline(ss, name)) {
      if (name.empty()) {
        continue;
      }
      name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
      auto match = std::find_if(
        sFrames.rbegin(), sFrames.rend(), [&name](const std::shared_ptr<DebugFrame>& f) {
          return f->hasVariable(name);
        });
      if (match == sFrames.rend()) {
        std::cout << "Unknown variable: " << name << std::endl;
        continue;
      }
      fs::path path = (*match)->varfilepath(name);
      if (fs::exists(path) && !fs::is_directory(path)) {
        Bytes    bytes = Bytes::loadFromFile(path);
        uint32_t typeId;
        bytes >> typeId;
        Bytes nested;
        bytes.readNested(nested);
        manager::watch(typeId, nested, name);
        std::cout << "Loading the debug geometry from path: " << path << std::endl;
      }
      else {
        std::cout << "Cannot load file: " << path << std::endl;
      }
    }
  }

  static void stackChanged() { instance()->setEdited(); }

protected:
  void handleChanges() override
  {
    if (!isEdited())
      return;
    reloadChanges();
    clearEdited();
  }
};

using FrameData = std::pair<std::string, uint64_t>;

static void updateFrames(const std::vector<FrameData>& frames)
{
  auto newIt = frames.begin();
  auto oldIt = sFrames.begin();

  while (newIt != frames.end() && oldIt != sFrames.end()) {
    const auto& newF = *newIt;
    const auto& oldF = *oldIt;
    newIt++;
    oldIt++;
    if (newF.first != oldF->name() || newF.second != oldF->id()) {
      break;
    }
  }

  auto oldIt2 = oldIt;
  while (oldIt2 != sFrames.end()) {
    debugPanel().removeWidget(*(oldIt2++));
  }
  sFrames.erase(oldIt, sFrames.end());

  size_t nNewFrames = std::distance(newIt, frames.end());
  std::transform(
    newIt, frames.end(), std::back_inserter(sFrames), [](const FrameData& f) {
      return debugPanel().newWidget<DebugFrame>(f.first, f.second);
    });
}

void loadCallstack()
{
  static std::vector<FrameData> sFrameCache;

  std::ifstream     stackfile(stackfilepath(), std::ios::in);
  std::string       line;
  FrameData         frameData;
  std::stringstream ss;
  while (std::getline(stackfile, line)) {
    if (line.empty()) {
      continue;
    }
    ss << line;
    ss >> frameData.first >> frameData.second;
    sFrameCache.emplace_back(frameData.first, frameData.second);
    ss.clear();
  }

  updateFrames(sFrameCache);
  sFrameCache.clear();
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
    loadCallstack();
    VariablesBox::stackChanged();
  }
};

void initSession(const fs::path& absDirPath)
{
  sDebugDirPath = absDirPath;
  if (!fs::exists(stackfilepath())) {
    throw std::runtime_error("Cannot find the stack file");
  }
  VariablesBox::init();
  loadCallstack();
  outputsPanel();

  // Start watching the folder.
  sFileWatcher  = std::make_shared<efsw::FileWatcher>();
  sFileListener = std::make_shared<FSListener>();
  std::cout << "Watching the folder: " << stackfiledir().string() << std::endl;
  sWatchId = sFileWatcher->addWatch(stackfiledir().string(), sFileListener.get(), true);
  sFileWatcher->watch();
}

}  // namespace debug
}  // namespace gal
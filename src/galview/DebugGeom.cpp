#include <galcore/DebugProfile.h>
#include <galcore/PointCloud.h>
#include <galcore/Serialization.h>
#include <galcore/Types.h>
#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/DebugGeom.h>
#include <galview/Widget.h>
#include <atomic>
#include <chrono>
#include <efsw/efsw.hpp>
#include <fstream>
#include <mutex>

namespace gal {
namespace debug {

class DebugFrame;
class FSListener;

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

static std::string                              sDebugDirPath;
static std::vector<std::shared_ptr<DebugFrame>> sFrames;
static std::shared_ptr<FSListener>              sFileListener;
static std::shared_ptr<efsw::FileWatcher>       sFileWatcher;
static efsw::WatchID                            sWatchId;

static std::unordered_map<std::string, bool>                          sFileChangedMap;
static std::unordered_map<std::string, std::shared_ptr<view::Widget>> sVarWidgetMap;
static std::unordered_map<std::string, size_t>                        sVarDrawIdMap;
static std::unordered_map<std::string, std::string>                   sVarFileMap;

static std::mutex sMutex;

static void setFileChanged(const std::string& filename, bool value)
{
  sFileChangedMap[filename] = value;
}

static bool setVarFile(const std::string& varname, const std::string& filename)
{
  auto match = sVarFileMap.find(varname);
  if (match == sVarFileMap.end()) {
    sVarFileMap.emplace(varname, filename);
    return true;
  }
  else if (match->second == filename) {
    return false;
  }
  else {
    match->second = filename;
    return true;
  }
}

static bool isFileChanged(const std::string& filename)
{
  auto match = sFileChangedMap.find(filename);
  if (match == sFileChangedMap.end())
    return true;
  return match->second;
}

static void setVarWidget(const std::string&                   varname,
                         const std::shared_ptr<view::Widget>& w)
{
  if (w) {
    sVarWidgetMap[varname] = w;
  }
};

static void setVarDrawId(const std::string& varname, size_t drawId)
{
  if (drawId) {
    sVarDrawIdMap[varname] = drawId;
  }
}

static void removeVarWidget(const std::string& varname)
{
  auto match = sVarWidgetMap.find(varname);
  if (match == sVarWidgetMap.end()) {
    return;
  }
  outputsPanel().removeWidget(match->second);
  sVarDrawIdMap.erase(varname);
}

static void removeVarDrawable(const std::string& varname)
{
  auto match = sVarDrawIdMap.find(varname);
  if (match == sVarDrawIdMap.end()) {
    return;
  }
  view::Context::get().removeDrawable(match->second);
  sVarDrawIdMap.erase(varname);
}

static fs::path stackfiledir()
{
  return sDebugDirPath / fs::path(sDebugDir);
}

static fs::path stackfilepath()
{
  return stackfiledir() / fs::path(sCallStackFile);
}

template<typename T, typename... TRest>
struct WatchManager
{
  static_assert(TypeInfo<T>::value, "Not a known type.");

  static void watch(uint32_t                       typeId,
                    Bytes&                         bytes,
                    const std::string&             geomKey,
                    size_t&                        drawId,
                    std::shared_ptr<view::Widget>& widget)
  {
    try {
      if (typeId == TypeInfo<T>::id) {
        if constexpr (view::MakeDrawable<T>::value) {
          auto checkBox = outputsPanel().newWidget<view::CheckBox>(geomKey, true);
          drawId        = view::Context::get().addDrawable(Serial<T>::deserialize(bytes),
                                                    checkBox->checkedPtr());
          widget        = checkBox;
        }
        else {
          T instance;
          bytes >> instance;
          std::stringstream ss;
          ss << instance;
          drawId = 0;
          widget = outputsPanel().newWidget<view::Text>(geomKey + ": " + ss.str());
        }
      }
      else if constexpr (sizeof...(TRest) > 0) {
        WatchManager<TRest...>::watch(typeId, bytes, geomKey, drawId, widget);
      }
      else if constexpr (sizeof...(TRest) == 0) {
        std::cerr << "Datatype " << gal::TypeInfo<T>::name
                  << " is not watchable in a debug session\n";
        throw std::bad_cast();
      }
    }
    catch (std::exception e) {
      std::cerr << "Cannot watch variable because of error: " << e.what() << std::endl;
    }
  }
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
    return stackfiledir() / fs::path(mPrefix + name + sDbgExt);
  }

  bool hasVariable(const std::string& name) const
  {
    return fs::exists(varfilepath(name));
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

static void loadCallstack()
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

static std::atomic_bool                                   sStackChanged = false;
static std::atomic_bool                                   sReloadStack  = true;
static std::atomic<std::chrono::system_clock::time_point> sLastReloadRequestTime =
  std::chrono::system_clock::now();

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

private:
  static void reloadChanges()
  {
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
        removeVarDrawable(name);
        removeVarWidget(name);
        continue;
      }
      fs::path    path    = (*match)->varfilepath(name);
      std::string fileKey = path.filename().string();
      if (!(setVarFile(name, fileKey) || isFileChanged(fileKey))) {
        // std::cout << "Variable " << name << " is uptodate.\n";
        continue;
      }
      removeVarDrawable(name);
      removeVarWidget(name);
      if (fs::exists(path) && !fs::is_directory(path)) {
        Bytes    bytes = Bytes::loadFromFile(path);
        uint32_t typeId;
        bytes >> typeId;
        Bytes nested;
        bytes.readNested(nested);

        std::cout << "Loading the debug geometry from path: " << path << std::endl;
        size_t                        drawId = 0;
        std::shared_ptr<view::Widget> widget;
        manager::watch(typeId, nested, name, drawId, widget);
        setFileChanged(fileKey, false);
        setVarDrawId(name, drawId);
        setVarWidget(name, widget);
      }
      else {
        std::cout << "Cannot load file: " << path << std::endl;
      }
    }
  }

protected:
  void handleChanges() override
  {
    if (!isEdited() && !sStackChanged)
      return;

    using namespace std::chrono_literals;
    if (std::chrono::system_clock::now() -
          std::chrono::system_clock::time_point(sLastReloadRequestTime) <
        200ms)
      return;

    std::lock_guard<std::mutex> lock(sMutex);
    if (sReloadStack) {
      loadCallstack();
      sReloadStack = false;
    }
    reloadChanges();
    clearEdited();
    sStackChanged = false;
  }
};

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
    static const fs::path fext(sDbgExt);
    static const fs::path fextTemp(sDbgExtTemp);
    fs::path              fpath(filename);
    if (fpath.extension() == fextTemp) {
      return;
    }
    else if (fpath.extension() == fext) {
      std::lock_guard<std::mutex> lock(sMutex);
      setFileChanged(filename, true);
      sStackChanged          = true;
      sLastReloadRequestTime = std::chrono::system_clock::now();
    }
    else if (filename == sCallStackFile) {
      std::lock_guard<std::mutex> lock(sMutex);
      sStackChanged          = true;
      sReloadStack           = true;
      sLastReloadRequestTime = std::chrono::system_clock::now();
    }
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
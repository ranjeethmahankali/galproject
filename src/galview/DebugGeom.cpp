#include <galcore/DebugProfile.h>
#include <galcore/PointCloud.h>
#include <galcore/Serialization.h>
#include <galcore/Types.h>
#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/DebugGeom.h>
#include <galview/Widget.h>
#include <fstream>

namespace gal {
namespace debug {

static std::string sDebugDirPath;

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

using manager = WatchManager<glm::vec2, Circle2d>;

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

protected:
  void handleChanges() override
  {
    if (!isEdited())
      return;
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
        manager::watch(typeId, bytes, name);
        std::cout << "Fake loading the debug geometry from path: " << path << std::endl;
      }
      else {
        std::cout << "Cannot find the file: " << path << std::endl;
      }
    }
    clearEdited();
  }
};

static fs::path stackfilepath()
{
  return sDebugDirPath / fs::path(sDebugDir) / fs::path(sCallStackFile);
}

void initSession(const fs::path& dirpath)
{
  sDebugDirPath = dirpath;
  if (!fs::exists(stackfilepath())) {
    throw std::runtime_error("Cannot find the stack file");
  }
  loadCallstack();
}

static void pushFrame(const std::string& name, uint64_t id)
{
  debugPanel().newWidget<DebugFrame>(name, id);
}

void clearCallstack()
{
  debugPanel().clearWidgets();
  outputsPanel().clearWidgets();
  view::Context::get().clearDrawables();
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
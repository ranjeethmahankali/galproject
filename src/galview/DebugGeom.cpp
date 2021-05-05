#include <galcore/DebugProfile.h>
#include <galcore/PointCloud.h>
#include <galcore/Serialization.h>
#include <galcore/Types.h>
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

template<typename T, typename... TRest>
struct DrawableManager
{
  static_assert(TypeInfo<T>::value, "Not a known type.");
  static_assert(Serial<T>::value, "Cannot serialize or deserialize this type.");

  static void draw(uint32_t typeId, Bytes& bytes, const std::string& geomKey)
  {
    if (typeId == TypeInfo<T>::id) {
      view::Context::addDrawable(
        Serial<T>::deserialize(bytes),
        debugPanel().newWidget<view::CheckBox>(geomKey, true)->checkedPtr());
    }
    else if constexpr (sizeof...(TRest) > 0) {
      DrawableManager<TRest...>::draw(typeId, bytes);
    }
  };
};

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
        std::cout << "Fake loading the debug geometry from path: " << path << std::endl;
      }
      else {
        std::cout << "Cannot find the file: " << path << std::endl;
      }
    }
    clearEdited();
  }
};

using manager = DrawableManager<PointCloud>;

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
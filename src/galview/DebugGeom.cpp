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

using manager = DrawableManager<PointCloud>;

static std::string sDebugDirPath;

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

static void pushFrame(const std::pair<std::string, uint64_t>& frame)
{
  debugPanel().newWidget<gal::view::Text>(frame.first + ": " +
                                          std::to_string(frame.second));
}

void clearCallstack()
{
  debugPanel().clearWidgets();
}

void loadCallstack()
{
  std::ifstream stackfile(stackfilepath(), std::ios::in);

  std::pair<std::string, uint64_t> frame;
  while (!stackfile.eof()) {
    stackfile >> frame.first >> frame.second;
    pushFrame(frame);
  }
}

}  // namespace debug
}  // namespace gal
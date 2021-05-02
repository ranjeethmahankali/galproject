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
  static view::Panel sDebugPanel = view::newPanel("Debug Geometry");
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

static std::string debugDirPath;

void initSession(const fs::path& dirpath)
{
  auto stackfile = dirpath / fs::path(sCallStackFile);
  if (!fs::exists(stackfile)) {
    throw std::runtime_error("Cannot find the stack file");
  }

  debugDirPath = dirpath;
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

void loadCallstack(const fs::path& dirpath)
{
  std::ifstream stackfile(dirpath / fs::path(sCallStackFile), std::ios::in);

  std::pair<std::string, uint64_t> frame;
  while (!stackfile.eof()) {
    stackfile >> frame.first >> frame.second;
    pushFrame(frame);
  }
}

}  // namespace debug
}  // namespace gal
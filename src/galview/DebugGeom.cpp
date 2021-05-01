#include <galcore/DebugProfile.h>
#include <galcore/Serialization.h>
#include <galcore/Types.h>
#include <galview/Context.h>
#include <galview/DebugGeom.h>
#include <galview/Widget.h>

#include <galcore/PointCloud.h>

namespace gal {
namespace debug {

view::Panel& debugPanel()
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

void DebugGeom::draw(uint64_t contextId)
{
  Bytes    bytes(fs::path(sDebugDir) / fs::path(std::to_string(contextId)));
  uint32_t typeId = 0;
  bytes >> typeId;
}

}  // namespace debug
}  // namespace gal
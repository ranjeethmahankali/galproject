#pragma once

#include <variant>
#include <vector>

#include <galview/AnnotationsView.h>
#include <galview/BoxView.h>
#include <galview/Circle2dView.h>
#include <galview/LineView.h>
#include <galview/MeshView.h>
#include <galview/PlaneView.h>
#include <galview/PointCloudView.h>
#include <galview/SphereView.h>

namespace gal {
namespace view {

template<typename T, typename... Ts>
class TViewManager
{
public:
  using DrawableVariantT = std::variant<Drawable<T>, Drawable<Ts>...>;

  template<typename U>
  static constexpr bool IsManagedType =
    std::is_same_v<T, U> || TViewManager<Ts...>::template IsManagedType<U>;
};

template<typename T>
class TViewManager<T>
{
public:
  using DrawableVariantT = std::variant<T>;

  template<typename U>
  static constexpr bool IsManagedType = std::is_same_v<U, T>;
};

using ViewManager = TViewManager<TextAnnotations,
                                 GlyphAnnotations,
                                 Box3,
                                 Circle2d,
                                 Line2d,
                                 Line3d,
                                 Mesh,
                                 Plane,
                                 PointCloud,
                                 Sphere>;

class Views
{
public:
  using VariantT   = typename ViewManager::DrawableVariantT;
  using RenderData = std::tuple<VariantT, const bool*, size_t>;

  static void remove(size_t id);

  static void render();

  static Box3 visibleBounds();

  static void clear();

private:
  static std::vector<RenderData> mDrawables;

  static size_t addInternal(VariantT&& view, const bool* vis);

public:
  template<typename T>
  static size_t add(const T& obj, const bool* visibility, size_t oldId = 0)
  {
    if constexpr (Drawable<T>::value && ViewManager::IsManagedType<T>) {
      remove(oldId);
      return addInternal(Drawable<T>(obj), visibility);
    }
    else {
      return 0;
    }
  }
};

}  // namespace view
}  // namespace gal

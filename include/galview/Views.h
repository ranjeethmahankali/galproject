#pragma once

#include <type_traits>
#include <variant>
#include <vector>

#include <galcore/Types.h>
#include <galview/AnnotationsView.h>
#include <galview/BoxView.h>
#include <galview/Circle2dView.h>
#include <galview/Context.h>
#include <galview/LineView.h>
#include <galview/MeshView.h>
#include <galview/PlaneView.h>
#include <galview/PointCloudView.h>
#include <galview/PointView.h>
#include <galview/SphereView.h>

namespace gal {
namespace view {

/**
 * @brief Templates that manages all data types that can be rendered in the viewer.
 *
 * @tparam T
 * @tparam Ts
 */
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

// Instantiation with all the actual types we care to render in the viewer.
using ViewManager = TViewManager<TextAnnotations,
                                 GlyphAnnotations,
                                 Box3,
                                 Circle2d,
                                 Line2d,
                                 Line3d,
                                 Mesh,
                                 glm::vec3,
                                 glm::vec2,
                                 Plane,
                                 PointCloud,
                                 Sphere>;

/**
 * @brief Helper class for managing objects in the viewer.
 *
 */
class Views
{
public:
  using VariantT = typename ViewManager::DrawableVariantT;
  struct RenderData
  {
    VariantT    mDrawable;
    const bool* mVisibility;

    RenderData(VariantT drawable, const bool* visibility);
  };

  template<typename T>
  static constexpr bool  IsDrawableType =
    Drawable<T>::value&& ViewManager::IsManagedType<T>;

  /**
   * @brief Render all objects currently in the scene. Ideally this should be called once
   * per-frame.
   *
   */
  static void render();

  /**
   * @brief Gets the bounds of all objects in the scene that are currently visible.
   *
   * @return Box3
   */
  static Box3 visibleBounds();

  /**
   * @brief Remove all objects from the scene.
   *
   */
  static void clear();

private:
  static size_t    addInternal(VariantT&& view, const bool* vis);
  static VariantT& getDrawable(size_t index);

public:
  template<typename T>
  static void update(size_t drawableIndex, const std::vector<T>& objs)
  {
    std::get<Drawable<T>>(getDrawable(drawableIndex)).update(objs);
  }

  /**
   * @brief Creates an new empty drawable of the given datatype.
   *
   * @tparam T The type of object to be drawn.
   * @param visibility The flag that controls the visibility of these objects. For
   * example, this can be linked to a UI control that the user can use to toggle the
   * visbility of these objects.
   * @return size_t Index of the drawable.
   */
  template<typename T>
  static size_t create(const bool* visibility)
  {
    static_assert(IsDrawableType<T>, "Must be a drawable type");
    return addInternal(Drawable<T>(), visibility);
  }
};

}  // namespace view
}  // namespace gal

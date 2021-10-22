#pragma once

#include <type_traits>
#include <variant>
#include <vector>

#include <galview/AnnotationsView.h>
#include <galview/BoxView.h>
#include <galview/Circle2dView.h>
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
  using VariantT   = typename ViewManager::DrawableVariantT;
  using RenderData = std::tuple<VariantT, const bool*, size_t>;

  /**
   * @brief Remove the object with the given id from the scene.
   *
   * @param id
   */
  static void remove(size_t id);

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
  static std::vector<RenderData> mDrawables;

  static size_t addInternal(VariantT&& view, const bool* vis);

public:
  /**
   * @brief Add a vector of objects of the given type to the scene, under a single
   * scene-object-id. If the datatype cannot be rendered in the viewer, this will do
   * nothing.
   *
   * @tparam T The type of object.
   * @param objs The objects.
   * @param visibility The flag that controls the visibility of these objects. For
   * example, this can be linked to a UI control that the user can use to toggle the
   * visbility of these objects.
   * @param oldId If the new objects need to replace old objects (for example, when
   * replacing the old outputs of a function with new ones), supplying this old id, will
   * cause the old objects to be removed from the scene before the new ones are added.
   * @return size_t The id corresponding to the added objects.
   */
  template<typename T>
  static size_t add(const std::vector<SafeInstanceType<T>>& objs,
                    const bool*                             visibility,
                    size_t                                  oldId = 0)
  {
    if constexpr (Drawable<T>::value && ViewManager::IsManagedType<T>) {
      remove(oldId);
      return addInternal(Drawable<T>(objs), visibility);
    }
    else {
      return 0;
    }
  }
};

}  // namespace view
}  // namespace gal

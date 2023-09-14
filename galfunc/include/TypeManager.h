#pragma once

#include <Converter.h>
#include <Data.h>
#include <MapMacro.h>
#include <Types.h>

#define GAL_EXTERN_TEMPLATE(T)                                                       \
  extern template class gal::func::data::Tree<T>;                                    \
  extern template struct gal::func::data::ReadView<T, 1>;                            \
  extern template struct gal::func::data::ReadView<T, 2>;                            \
  extern template struct gal::func::data::WriteView<T, 1>;                           \
  extern template struct gal::func::data::WriteView<T, 2>;                           \
  extern template struct gal::func::data::repeat::CombiView<T, true>;                \
  extern template struct gal::func::data::repeat::CombiView<T, false>;               \
  extern template struct gal::func::Converter<gal::func::data::Tree<T>, py::object>; \
  extern template struct gal::func::Converter<py::object, gal::func::data::Tree<T>>; \
  extern template struct gal::func::Converter<py::object, T>;                        \
  extern template struct gal::func::Converter<T, py::object>;                        \
  extern template struct gal::func::Converter<py::list, std::vector<T>>;             \
  extern template struct gal::func::Converter<std::vector<T>, py::list>;

#define GAL_MANAGED_TYPES                                                             \
  uint8_t, int32_t, uint64_t, float, gal::Bool, std::string, glm::vec3, glm::vec2,    \
    gal::Sphere, gal::Plane, gal::Box3, gal::Box2, gal::PointCloud<3>, gal::Circle2d, \
    gal::Line2d, gal::Line3d, gal::TriMesh, gal::PolyMesh, gal::TextAnnotations,      \
    gal::Glyph, gal::GlyphAnnotations

namespace gal {
namespace func {

/**
 * @brief Helper template to manage all the recognized datatypes.
 *
 * @tparam T The first datatype.
 * @tparam Ts The remaining datatypes.
 */
template<typename... Ts>
struct TypeManager
{
  /**
   * @brief Calls the static "invoke" function of the invoker type on
   * all the types, one after another recursively.
   *
   * @tparam InvokerType Class template with 1 typename template argument.
   */
  template<template<typename> typename InvokerType, typename... TArgs>
  static void invoke(TArgs... args)
  {
    return (InvokerType<Ts>::invoke(args...), ...);
  }
};

// Instantiation that manages all the known recognized types.
using typemanager = TypeManager<GAL_MANAGED_TYPES>;

// Extern instantiations of templates of all managed types to avoid recompilation.
MAP(GAL_EXTERN_TEMPLATE, GAL_MANAGED_TYPES)

}  // namespace func
}  // namespace gal

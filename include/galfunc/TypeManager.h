#pragma once
#include <galcore/Types.h>
#include <galfunc/Data.h>
#include <galfunc/MapMacro.h>

#define GAL_EXTERN_TEMPLATE(type)                             \
  extern template class gal::func::data::Tree<type>;          \
  extern template struct gal::func::data::ReadView<type, 1>;  \
  extern template struct gal::func::data::ReadView<type, 2>;  \
  extern template struct gal::func::data::WriteView<type, 1>; \
  extern template struct gal::func::data::WriteView<type, 2>; \
  extern template struct gal::func::data::repeat::CombiView<type>;

#define GAL_MANAGED_TYPES                                                          \
  uint8_t, int32_t, uint64_t, float, gal::Bool, std::string, glm::vec3, glm::vec2, \
    gal::Sphere, gal::Plane, gal::Box3, gal::Box2, gal::PointCloud, gal::Circle2d, \
    gal::Line2d, gal::Line3d, gal::Mesh, gal::TextAnnotations, gal::Glyph,         \
    gal::GlyphAnnotations

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
  template<template<typename> typename InvokerType>
  static void invoke()
  {
    return (InvokerType<Ts>::invoke(), ...);
  }
};

// Instantiation that manages all the known recognized types.
using typemanager = TypeManager<GAL_MANAGED_TYPES>;

// Explicitly instantiate data trees of all managed types.
MAP(GAL_EXTERN_TEMPLATE, GAL_MANAGED_TYPES)

}  // namespace func
}  // namespace gal

#pragma once
#include <galcore/Types.h>
#include <galfunc/Data.h>

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
using typemanager = TypeManager<uint8_t,
                                int32_t,
                                uint64_t,
                                float,
                                gal::Bool,
                                std::string,
                                glm::vec3,
                                glm::vec2,
                                gal::Sphere,
                                gal::Plane,
                                gal::Box3,
                                gal::Box2,
                                gal::PointCloud,
                                gal::Circle2d,
                                gal::Line2d,
                                gal::Line3d,
                                gal::Mesh,
                                gal::TextAnnotations,
                                gal::Glyph,
                                gal::GlyphAnnotations>;

}  // namespace func
}  // namespace gal

#pragma once
#include <galcore/Types.h>

namespace gal {
namespace func {

/**
 * @brief Helper template to manage all the recognized datatypes.
 *
 * @tparam T The first datatype.
 * @tparam Ts The remaining datatypes.
 */
template<typename T, typename... Ts>
struct TypeManager
{
  static constexpr size_t NumTypes = 1 + sizeof...(Ts);

  /**
   * @brief Calls the static "invoke" function of the invoker type on
   * all the types, one after another recursively.
   *
   * @tparam InvokerType Class template with 1 typename template argument.
   */
  template<template<typename> typename InvokerType>
  static void invoke()
  {
    InvokerType<T>::invoke();
    if constexpr (sizeof...(Ts) > 0) {
      TypeManager<Ts...>::template invoke<InvokerType>();
    }
  }
};

// Instantiation that manages all the known recognized types.
using typemanager = TypeManager<uint8_t,
                                int32_t,
                                uint64_t,
                                float,
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

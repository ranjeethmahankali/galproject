#pragma once

#include <galcore/Annotations.h>
#include <galcore/Box.h>
#include <galcore/Circle2d.h>
#include <galcore/Line.h>
#include <galcore/Mesh.h>
#include <galcore/Plane.h>
#include <galcore/PointCloud.h>
#include <galcore/Sphere.h>
#include <galcore/Traits.h>

namespace gal {

template<typename T>
struct TypeInfo : public std::false_type
{
  static constexpr uint32_t id      = 0U;
  static constexpr char     sName[] = "UnknownType";
  static std::string        name() noexcept { return std::string(sName); }
};

template<typename T>
struct TypeInfo<const T> : public TypeInfo<T>
{
};

}  // namespace gal

#define GAL_TYPE_INFO(type, idInt)                                           \
  template<>                                                                 \
  struct gal::TypeInfo<type> : public std::true_type                         \
  {                                                                          \
    static constexpr uint32_t id      = idInt;                               \
    static constexpr char     sName[] = #type;                               \
    static std::string        name() noexcept { return std::string(sName); } \
  };

namespace gal {

template<typename T>
struct TypeInfo<std::vector<T>> : public std::true_type
{
  static_assert(TypeInfo<T>::value);
  static constexpr uint32_t sVecMask   = 0xe2b7b4b9;
  static constexpr char     sVecName[] = "vec_";
  static constexpr uint32_t id         = TypeInfo<T>::id ^ sVecMask;

  static std::string name() noexcept
  {
    return std::string(sVecName) + TypeInfo<T>::name();
  }
};
}  // namespace gal

GAL_TYPE_INFO(void, 0x9267e7bf);
GAL_TYPE_INFO(bool, 0x9566a7b1);
GAL_TYPE_INFO(int32_t, 0x9234a3b1);
GAL_TYPE_INFO(uint64_t, 0x913eb3be);
GAL_TYPE_INFO(float, 0x32542672);
GAL_TYPE_INFO(std::string, 0x12340989);

GAL_TYPE_INFO(glm::vec3, 0x33821151);
GAL_TYPE_INFO(glm::vec2, 0xbd40c8a1);
GAL_TYPE_INFO(gal::Sphere, 0x5e8f631c);
GAL_TYPE_INFO(gal::Plane, 0x591f323f);
GAL_TYPE_INFO(gal::Box3, 0x8fcb9e01);
GAL_TYPE_INFO(gal::Box2, 0xd60b396d);
GAL_TYPE_INFO(gal::PointCloud, 0xe6e934eb);
GAL_TYPE_INFO(gal::Circle2d, 0x3271dc29);
GAL_TYPE_INFO(gal::Line2d, 0x34ff4158);
GAL_TYPE_INFO(gal::Line3d, 0x989fdbdd);
GAL_TYPE_INFO(gal::Mesh, 0x45342367);
GAL_TYPE_INFO(gal::TextAnnotations, 0x901da902);
GAL_TYPE_INFO(gal::Glyph, 0x71b91b77);
GAL_TYPE_INFO(gal::GlyphAnnotations, 0x15301102);

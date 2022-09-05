#pragma once

#include <Annotations.h>
#include <Box.h>
#include <Circle2d.h>
#include <Line.h>
#include <Mesh.h>
#include <Plane.h>
#include <PointCloud.h>
#include <Sphere.h>
#include <Traits.h>

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

#define GAL_TYPE_INFO(type, typeName, idInt)                                        \
  template<>                                                                        \
  struct gal::TypeInfo<gal::RemoveBraces<void(type)>::Type> : public std::true_type \
  {                                                                                 \
    static constexpr uint32_t id      = idInt;                                      \
    static constexpr char     sName[] = #typeName;                                  \
    static std::string        name() noexcept { return std::string(sName); }        \
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

GAL_TYPE_INFO(uint8_t, uint8, 0x313861ed);
GAL_TYPE_INFO(int32_t, int, 0x9234a3b1);
GAL_TYPE_INFO(uint64_t, uint64, 0x913eb3be);
GAL_TYPE_INFO(float, float, 0x32542672);
GAL_TYPE_INFO(std::string, string, 0x12340989);

GAL_TYPE_INFO(glm::vec3, vec3, 0x33821151);
GAL_TYPE_INFO(glm::vec2, vec2, 0xbd40c8a1);
GAL_TYPE_INFO(gal::Sphere, sphere, 0x5e8f631c);
GAL_TYPE_INFO(gal::Plane, plane, 0x591f323f);
GAL_TYPE_INFO(gal::Box3, box3, 0x8fcb9e01);
GAL_TYPE_INFO(gal::Box2, box2, 0xd60b396d);
GAL_TYPE_INFO(gal::PointCloud<3>, ptcloud, 0xe6e934eb);
GAL_TYPE_INFO(gal::Circle2d, circle2d, 0x3271dc29);
GAL_TYPE_INFO(gal::Line2d, line2d, 0x34ff4158);
GAL_TYPE_INFO(gal::Line3d, line3d, 0x989fdbdd);
GAL_TYPE_INFO(gal::TriMesh, mesh, 0x45342367);
GAL_TYPE_INFO(gal::TextAnnotations, tags, 0x901da902);
GAL_TYPE_INFO(gal::Glyph, glyphIcon, 0x71b91b77);
GAL_TYPE_INFO(gal::GlyphAnnotations, glyph, 0x15301102);

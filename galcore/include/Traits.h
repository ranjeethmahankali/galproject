#pragma once

#include <memory>
#include <ostream>
#include <type_traits>

#include <glm/glm.hpp>

namespace gal {
template<typename T>
struct IsValueType : public std::is_fundamental<T>
{
};

template<>
struct IsValueType<glm::vec3> : public std::true_type
{
};

template<>
struct IsValueType<glm::vec2> : public std::true_type
{
};

// Can be used to check at compile time if a type is a template instantiation.
template<template<typename...> typename Tem, typename T>
struct IsInstance : public std::false_type
{
};

// Template specialization that does the magic.
template<template<typename...> typename Tem, typename... Ts>
struct IsInstance<Tem, Tem<Ts...>> : public std::true_type
{
};

template<template<typename...> typename Tem, typename... Ts>
struct IsInstance<Tem, const Tem<Ts...>> : public std::true_type
{
};

template<typename T>
struct GlmVecTraits
{
  static constexpr bool IsGlmVec = false;
  static constexpr int  Size     = 0;
  using ValueType                = void;
};

template<int N, typename T, glm::qualifier Q>
struct GlmVecTraits<glm::vec<N, T, Q>>
{
  static constexpr bool IsGlmVec = true;
  static constexpr int  Size     = N;
  using ValueType                = T;
};

/**
 * @brief Checks if the type is streamable to std::ostream.
 *
 * @tparam T The type to be checked.
 */
template<typename T>
class IsPrintable
{
  template<typename SS, typename TT>
  static auto Check(int)
    -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

  template<typename, typename>
  static auto Check(...) -> std::false_type;

public:
  static const bool value = decltype(Check<std::ostream, T>(0))::value;
};

/** You can't have polymorphic instances on the stack. Its inefficient to have
 * non-polymorphic instances on the heap. This helper template can decide where to put
 * what.
 */
template<typename U>
using SafeInstanceType =
  std::conditional_t<std::is_polymorphic_v<U>, std::shared_ptr<U>, U>;

}  // namespace gal

#pragma once

#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/Utils/vector_traits.hh>
#include <glm/glm.hpp>
#include <type_traits>

namespace OpenMesh {

template<typename T, int N, glm::qualifier Q>
T norm(const glm::vec<N, T, Q>& v)
{
  return glm::length(v);
}

template<typename T, int N, glm::qualifier Q>
glm::vec<N, T, Q>& vectorize(glm::vec<N, T, Q>& v, T s)
{
  for (int i = 0; i < N; i++) {
    v[i] = s;
  }
  return v;
}

/**
 * @brief Partial specialization to tell OpenMesh about glm vectors.
 *
 * @tparam N
 * @tparam T
 * @tparam Q
 */
template<int N, typename T, glm::qualifier Q>
struct vector_traits<glm::vec<N, T, Q>>
{
  typedef glm::vec<N, T, Q> vector_type;
  typedef T                 value_type;
  static constexpr size_t   size_ = size_t(N);
  static constexpr size_t   size() { return size_; }
};

template<typename T>
struct color_caster<OpenMesh::VectorT<T, 4>, glm::vec3>
{
  typedef OpenMesh::VectorT<T, 4> return_type;

  inline static return_type cast(const glm::vec3& src)
  {
    return_type dst;
    for (int i = 0; i < 3; ++i) {
      if constexpr (std::is_integral_v<T>) {
        dst[i] = (T)(src[i] * 255);
      }
      else {
        dst[i] = T(src[i]);
      }
    }
    if constexpr (std::is_integral_v<T>) {
      dst[3] = 255;
    }
    else {
      dst[3] = T(1.);
    }
    return dst;
  }
};

}  // namespace OpenMesh

/* IMPORTANT: These need to be included for this code to compile! */
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#pragma once

#include <OpenMesh/Core/Utils/vector_traits.hh>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace OpenMesh {

template<typename T, int N, glm::qualifier Q>
T norm(const glm::vec<N, T, Q>& v)
{
  return glm::length(v);
}

template<typename T, int N, glm::qualifier Q>
T sqrnorm(const glm::vec<N, T, Q>& v)
{
  return glm::length2(v);
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

// These weird operators are used in some open mesh stuff.

inline glm::vec3 operator%(glm::vec3 const& a, glm::vec3 const& b)
{
  return glm::cross(a, b);
}

inline float operator|(glm::vec3 const& a, glm::vec3 const& b)
{
  return glm::dot(a, b);
}

}  // namespace OpenMesh

/* IMPORTANT: These need to be included for this code to compile! */
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

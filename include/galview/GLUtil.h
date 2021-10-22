#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#ifdef _MSVC
#else
#define DEBUG_BREAK __builtin_trap()
#endif

// #define GAL_GL_LOG
#if defined GAL_GL_LOG
#define GL_CALL(fncall)                                       \
  {                                                           \
    gal::glutil::clear_errors();                              \
    fncall;                                                   \
    if (gal::glutil::log_errors(#fncall, __FILE__, __LINE__)) \
      DEBUG_BREAK;                                            \
    std::cout << #fncall << ": " << __FILE__ << std::endl;    \
  }
#elif defined NDEBUG
#define GL_CALL(fncall) fncall
#else
#define GL_CALL(fncall)                                       \
  {                                                           \
    gal::glutil::clear_errors();                              \
    fncall;                                                   \
    if (gal::glutil::log_errors(#fncall, __FILE__, __LINE__)) \
      DEBUG_BREAK;                                            \
  }
#endif  // DEBUG

using uint = GLuint;

namespace gal {
namespace glutil {

bool log_errors(const char* function, const char* file, uint line);
void clear_errors();

struct DefaultVertex
{
  glm::vec3 position = glm::vec3 {0.f, 0.f, 0.f};
  glm::vec3 normal   = glm::vec3 {0.f, 0.f, 0.f};

  /**
   * @brief This will setup the vertex attribute layout. This must be called after the
   * appropriate buffers are bound.
   */
  static void initAttributes();
};

struct MeshVertex
{
  glm::vec3 position = glm::vec3 {0.f, 0.f, 0.f};
  glm::vec3 normal   = glm::vec3 {0.f, 0.f, 0.f};
  glm::vec3 color    = glm::vec3 {1.f, 1.f, 1.f};

  /**
   * @brief This will setup the vertex attribute layout. This must be called after the
   * appropriate buffers are bound.
   */
  static void initAttributes();
};

template<typename V>
class TVertexBuffer : public std::vector<V>
{
public:
  using VertexType = V;

  TVertexBuffer(size_t nverts)
      : std::vector<V>(nverts)
  {}

  using std::vector<V>::size;
  using std::vector<V>::data;

  size_t numbytes() const { return sizeof(V) * size(); }

  void finalize(uint32_t& vao, uint32_t& vbo) const
  {
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glGenBuffers(1, &vbo));

    GL_CALL(glBindVertexArray(vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, numbytes(), data(), GL_STATIC_DRAW));

    V::initAttributes();

    // Unbind stuff.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
  }
};

using VertexBuffer     = TVertexBuffer<DefaultVertex>;
using MeshVertexBuffer = TVertexBuffer<MeshVertex>;

class IndexBuffer : public std::vector<uint32_t>
{
public:
  IndexBuffer(size_t nIndices);

  size_t numbytes() const;
  void   finalize(uint32_t& ibo) const;
};

};  // namespace glutil
}  // namespace gal

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>

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
    gal::glutil::logger().debug("{}: {}", #fncall, __FILE__); \
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

spdlog::logger& logger();
bool            log_errors(const char* function, const char* file, uint line);
void            clear_errors();

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
  uint32_t mVAO = 0;
  uint32_t mVBO = 0;

  void free()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
      mVAO = 0;
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
      mVBO = 0;
    }
  }

public:
  using VertexType = V;

  TVertexBuffer() = default;

  TVertexBuffer(size_t nverts)
      : std::vector<V>(nverts)
  {}

  ~TVertexBuffer() { free(); }

  using std::vector<V>::size;
  using std::vector<V>::data;

  // Disallow copying.
  TVertexBuffer(const TVertexBuffer&) = delete;
  const TVertexBuffer& operator=(const TVertexBuffer&) = delete;

  const TVertexBuffer& operator=(TVertexBuffer&& other)
  {
    if (this != &other) {
      free();
      std::vector<V>::operator=(std::move(other));
      mVAO                    = std::exchange(other.mVAO, 0);
      mVBO                    = std::exchange(other.mVBO, 0);
    }
    return *this;
  }
  TVertexBuffer(TVertexBuffer&& other) { *this = std::move(other); }

  size_t numbytes() const { return sizeof(V) * size(); }

  /**
   * @brief Bind the vertex array.
   *
   */
  void bindVao() const { GL_CALL(glBindVertexArray(mVAO)); }

  /**
   * @brief Binds the vertex buffer.
   *
   */
  void bindVbo() const { GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO)); }

  /**
   * @brief Allocates the vertex array and vertex buffer on the GPU and copies the data.
   *
   */
  void alloc()
  {
    free();  // Free if already bound.
    GL_CALL(glGenVertexArrays(1, &mVAO));
    GL_CALL(glGenBuffers(1, &mVBO));

    bindVao();
    bindVbo();
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
  uint32_t mIBO = 0;

  void free();

public:
  IndexBuffer() = default;
  IndexBuffer(size_t nIndices);
  IndexBuffer(IndexBuffer&& other);
  ~IndexBuffer();

  // Disallow copying.
  IndexBuffer(const IndexBuffer&) = delete;
  const IndexBuffer& operator=(const IndexBuffer&) = delete;

  const IndexBuffer& operator=(IndexBuffer&&);

  /**
   * @brief Allocates the index buffer on the GPU and copies the indices.
   *
   */
  void alloc();

  /**
   * @brief Binds the index buffer.
   *
   */
  void bind() const;
};

};  // namespace glutil
}  // namespace gal

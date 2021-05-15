#include <galview/GLUtil.h>
#include <iostream>

namespace gal {
namespace glutil {

bool log_errors(const char* function, const char* file, uint line)
{
  static bool found_error = false;
  while (GLenum error = glGetError()) {
    std::cout << "[OpenGL Error] (0x" << std::hex << error << std::dec << ")";
#ifdef _DEBUG
    std::cout << " in " << function << " at " << file << ":" << line;
#endif  // NDEBUG
    std::cout << std::endl;
    found_error = true;
  }
  if (found_error) {
    found_error = false;
    return true;
  }
  else {
    return false;
  }
};

void clear_errors()
{
  // Just loop over and consume all pending errors.
  GLenum error = glGetError();
  while (error) {
    error = glGetError();
  }
};

VertexBuffer::VertexBuffer(size_t nverts)
    : std::vector<Vertex>(nverts)
{}

size_t VertexBuffer::numbytes() const
{
  return sizeof(Vertex) * size();
}

void VertexBuffer::finalize(uint32_t& vao, uint32_t& vbo) const
{
  GL_CALL(glGenVertexArrays(1, &vao));
  GL_CALL(glGenBuffers(1, &vbo));

  GL_CALL(glBindVertexArray(vao));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, numbytes(), data(), GL_STATIC_DRAW));

  static constexpr size_t stride    = sizeof(Vertex);
  static const void*      posOffset = (void*)(&(((Vertex*)nullptr)->position));
  static const void*      nrmOffset = (void*)(&(((Vertex*)nullptr)->normal));
  static const void*      texOffset = (void*)(&(((Vertex*)nullptr)->texCoords));
  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, posOffset));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, nrmOffset));
  GL_CALL(glEnableVertexAttribArray(1));
  GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, texOffset));
  GL_CALL(glEnableVertexAttribArray(2));

  // Unbind stuff.
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  GL_CALL(glBindVertexArray(0));
}

IndexBuffer::IndexBuffer(size_t nIndices)
    : std::vector<uint32_t>(nIndices) {};

size_t IndexBuffer::numbytes() const
{
  return sizeof(uint32_t) * size();
}

void IndexBuffer::finalize(uint32_t& ibo) const
{
  GL_CALL(glGenBuffers(1, &ibo));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
  GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, numbytes(), data(), GL_STATIC_DRAW));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

}  // namespace glutil
}  // namespace gal

#include <galview/GLUtil.h>
#include <iostream>

namespace gal {
namespace glutil {

bool log_errors(const char* function, const char* file, uint line)
{
  static bool found_error = false;
  while (GLenum error = glGetError()) {
    std::cout << "[OpenGL Error] (0x" << std::hex << error << std::dec << ")";
#ifndef NDEBUG
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

void DefaultVertex::initAttributes()
{
  static constexpr size_t stride    = sizeof(DefaultVertex);
  static const void*      posOffset = (void*)(&(((DefaultVertex*)nullptr)->position));
  static const void*      nrmOffset = (void*)(&(((DefaultVertex*)nullptr)->normal));
  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, posOffset));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, nrmOffset));
  GL_CALL(glEnableVertexAttribArray(1));
}

void MeshVertex::initAttributes()
{
  static constexpr size_t stride    = sizeof(MeshVertex);
  static const void*      posOffset = (void*)(&(((MeshVertex*)nullptr)->position));
  static const void*      nrmOffset = (void*)(&(((MeshVertex*)nullptr)->normal));
  static const void*      clrOffset = (void*)(&(((MeshVertex*)nullptr)->color));
  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, posOffset));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, nrmOffset));
  GL_CALL(glEnableVertexAttribArray(1));
  GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, clrOffset));
  GL_CALL(glEnableVertexAttribArray(2));
}

void IndexBuffer::free()
{
  if (mIBO) {
    GL_CALL(glDeleteBuffers(1, &mIBO));
    mIBO = 0;
  }
}

IndexBuffer::IndexBuffer(size_t nIndices)
    : std::vector<uint32_t>(nIndices) {};

IndexBuffer::IndexBuffer(IndexBuffer&& other)
{
  *this = std::move(other);
}

IndexBuffer::~IndexBuffer()
{
  free();
}

const IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
{
  if (this != &other) {
    free();
    std::vector<uint32_t>::operator=(std::move(other));
    mIBO                           = std::exchange(other.mIBO, 0);
  }
  return *this;
}

void IndexBuffer::alloc()
{
  size_t nBytes = sizeof(uint32_t) * size();
  free();
  GL_CALL(glGenBuffers(1, &mIBO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, nBytes, data(), GL_STATIC_DRAW));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void IndexBuffer::bind() const
{
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
}

}  // namespace glutil
}  // namespace gal

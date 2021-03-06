#include <galview/MeshView.h>
#include <stdint.h>

using namespace gal;

MeshView MeshView::create(const Mesh& mesh)
{
  MeshView view;

  // 3 Coords per vertex and nothing else.
  view.mVSize = sizeof(float) * 3 * mesh.numVertices();
  std::vector<float> vBuf(3 * mesh.numVertices(), FLT_MAX);
  float*             dstf   = vBuf.data();
  auto               vbegin = mesh.vertexCBegin();
  auto               vend   = mesh.vertexCEnd();
  while (vbegin != vend) {
    const glm::vec3& v = *(vbegin++);
    *(dstf++)          = v.x;
    *(dstf++)          = v.y;
    *(dstf++)          = v.z;
  }

  // 3 indices per face and nothing else.
  view.mISize = sizeof(uint32_t) * 3 * mesh.numFaces();
  std::vector<uint32_t> iBuf(3 * mesh.numFaces());
  uint32_t*             dsti   = iBuf.data();
  auto                  fbegin = mesh.faceCBegin();
  auto                  fend   = mesh.faceCEnd();
  while (fbegin != fend) {
    const Mesh::Face& face = *(fbegin++);
    *(dsti++)              = (uint32_t)face.a;
    *(dsti++)              = (uint32_t)face.b;
    *(dsti++)              = (uint32_t)face.c;
  }

  // Now write the data to the device.
  GL_CALL(glGenVertexArrays(1, &view.mVao));
  GL_CALL(glGenBuffers(1, &view.mVBO));
  GL_CALL(glGenBuffers(1, &view.mIBO));

  GL_CALL(glBindVertexArray(view.mVao));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, view.mVBO));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, view.mVSize, vBuf.data(), GL_STATIC_DRAW));

  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, view.mIBO));
  GL_CALL(
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, view.mISize, iBuf.data(), GL_STATIC_DRAW));

  // Unbind stuff.
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  GL_CALL(glBindVertexArray(0));
  return view;
}

MeshView::~MeshView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVao));
  GL_CALL(glDeleteBuffers(1, &mVBO));
  GL_CALL(glDeleteBuffers(1, &mIBO));
}

void MeshView::draw() const
{
  GL_CALL(glBindVertexArray(mVao));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));
}
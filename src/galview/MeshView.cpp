#include <galview/MeshView.h>
#include <stdint.h>

using namespace gal;
using namespace gal::view;

MeshView MeshView::create(const Mesh& mesh)
{
  MeshView view;

  // 3 Coords per vertex and nothing else.
  size_t nVerts = mesh.numVertices();
  // Position and Normal for each vertex.
  std::vector<float> vBuf(6 * mesh.numVertices(), FLT_MAX);
  float*             dstf = vBuf.data();
  for (size_t i = 0; i < nVerts; i++) {
    glm::vec3 v = mesh.vertex(i);
    *(dstf++)   = v.x;
    *(dstf++)   = v.y;
    *(dstf++)   = v.z;
    v           = mesh.vertexNormal(i);
    *(dstf++)   = v.x;
    *(dstf++)   = v.y;
    *(dstf++)   = v.z;
  }
  view.mVSize = sizeof(float) * vBuf.size();

  // 3 indices per face and nothing else.
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
  view.mISize = sizeof(uint32_t) * iBuf.size();

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

  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));
  GL_CALL(glEnableVertexAttribArray(1));

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
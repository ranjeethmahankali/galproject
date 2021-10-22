#pragma once

#include <galcore/Mesh.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <array>

namespace gal {
namespace view {

template<>
struct Drawable<Mesh> : public std::true_type
{
  static constexpr glm::vec4 sFaceColor = {1.f, 1.f, 1.f, 1.f};
  static constexpr glm::vec4 sEdgeColor = {0.f, 0.f, 0.f, 1.f};

private:
  Box3 mBounds;
  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.a
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.

public:
  Drawable<Mesh>(const Mesh& mesh)
  {
    // Position and Normal for each vertex.
    glutil::MeshVertexBuffer vBuf(mesh.numVertices());
    auto                     vbegin  = vBuf.begin();
    size_t                   nVerts  = mesh.numVertices();
    const auto&              vColors = mesh.vertexColors();
    for (size_t i = 0; i < nVerts; i++) {
      *(vbegin++) = {mesh.vertex(i), mesh.vertexNormal(i), vColors[i]};
    }
    mVSize  = (uint32_t)vBuf.size();
    mBounds = mesh.bounds();

    // 3 indices per face and nothing else.
    glutil::IndexBuffer iBuf(3 * mesh.numFaces());
    uint32_t*           dsti   = iBuf.data();
    auto                fbegin = mesh.faceCBegin();
    auto                fend   = mesh.faceCEnd();
    while (fbegin != fend) {
      const Mesh::Face& face = *(fbegin++);
      *(dsti++)              = uint32_t(face.a);
      *(dsti++)              = uint32_t(face.b);
      *(dsti++)              = uint32_t(face.c);
    }
    mISize = (uint32_t)iBuf.size();

    vBuf.finalize(mVAO, mVBO);
    iBuf.finalize(mIBO);
  }

  ~Drawable<Mesh>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
    if (mIBO) {
      GL_CALL(glDeleteBuffers(1, &mIBO));
    }
  }

  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;

  const Drawable& operator=(Drawable&& other)
  {
    mBounds = other.mBounds;
    mVAO    = std::exchange(other.mVAO, 0);
    mVBO    = std::exchange(other.mVBO, 0);
    mIBO    = std::exchange(other.mIBO, 0);
    mVSize  = other.mVSize;
    mISize  = other.mISize;
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  Box3 bounds() const { return mBounds; }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = uint64_t(0x0000ff) |
                                 (uint64_t((1.f - sEdgeColor.a) * 255.f) << 8) |
                                 (uint64_t((1.f - sFaceColor.a) * 255.f) << 16);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.shaderId    = Context::get().shaderId("mesh");
    settings.faceColor   = sFaceColor;
    settings.edgeColor   = sEdgeColor;
    settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    return settings;
  }

  void draw() const
  {
    static RenderSettings rsettings = renderSettings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
    GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));

    if (Context::get().wireframeMode()) {
      rsettings.edgeColor   = {0.f, 0.f, 0.f, 1.f};
      rsettings.faceColor   = {0.f, 0.f, 0.f, 1.f};
      rsettings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_LINE);
      rsettings.apply();
      GL_CALL(glBindVertexArray(mVAO));
      GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
      GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));
    }
  }
};

}  // namespace view
}  // namespace gal

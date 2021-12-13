#pragma once

#include <galcore/Mesh.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <array>
#include <numeric>

namespace gal {
namespace view {

template<>
struct Drawable<Mesh> : public std::true_type
{
  static constexpr glm::vec4 sFaceColor      = {1.f, 1.f, 1.f, 1.f};
  static constexpr glm::vec4 sWireframeColor = {.8f, .8f, .8f, 1.f};
  static constexpr glm::vec4 sEdgeColor      = {0.f, 0.f, 0.f, 1.f};

private:
  glutil::MeshVertexBuffer mVBuf;
  glutil::IndexBuffer      mIBuf;
  Box3                     mBounds;

public:
  void update(const std::vector<Mesh>& meshes)
  {
    mBounds = gal::Box3();
    mVBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const Mesh& mesh) {
        return total + mesh.numVertices();
      }));
    mIBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const Mesh& mesh) {
        return total + 3 * mesh.numFaces();
      }));

    auto      vbegin = mVBuf.begin();
    uint32_t* dsti   = mIBuf.data();
    uint32_t  off    = 0;
    for (const auto& mesh : meshes) {
      size_t      nVerts  = mesh.numVertices();
      const auto& vColors = mesh.vertexColors();
      for (size_t i = 0; i < nVerts; i++) {
        *(vbegin++) = {mesh.vertex(i), mesh.vertexNormal(i), vColors[i]};
      }

      // 3 indices per face and nothing else.
      auto fbegin = mesh.faceCBegin();
      auto fend   = mesh.faceCEnd();
      while (fbegin != fend) {
        const Mesh::Face& face = *(fbegin++);
        *(dsti++)              = off + uint32_t(face.a);
        *(dsti++)              = off + uint32_t(face.b);
        *(dsti++)              = off + uint32_t(face.c);
      }
      off += uint32_t(mesh.numVertices());

      mBounds.inflate(mesh.bounds());
    }
    mVBuf.alloc();
    mIBuf.alloc();
  }

  Box3 bounds() const { return mBounds; }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = (uint64_t((1.f - sEdgeColor.a) * 255.f) << 8) |
                                 (uint64_t((1.f - sFaceColor.a) * 255.f) << 16);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.shaderId = Context::get().shaderId("mesh");
    return settings;
  }

  void draw() const
  {
    static RenderSettings settings;
    settings.edgeMode = false;
    mVBuf.bindVao();
    mIBuf.bind();
    if (Context::get().wireframeMode()) {
      settings.edgeMode    = true;
      settings.edgeColor   = sWireframeColor;
      settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
      if (Context::get().meshEdgeMode()) {
        GL_CALL(glEnable(GL_POLYGON_OFFSET_FILL));
        GL_CALL(glPolygonOffset(1, 1));
      }
      settings.faceColor   = sFaceColor;
      settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    }
    settings.apply();
    GL_CALL(glDrawElements(GL_TRIANGLES, mIBuf.size(), GL_UNSIGNED_INT, nullptr));

    if (!Context::get().wireframeMode() && Context::get().meshEdgeMode()) {
      settings.edgeColor   = sEdgeColor;
      settings.edgeMode    = true;
      settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_LINE);
      GL_CALL(glDisable(GL_POLYGON_OFFSET_FILL));
      settings.apply();
      GL_CALL(glDrawElements(GL_TRIANGLES, mIBuf.size(), GL_UNSIGNED_INT, nullptr));
    }
  }
};

}  // namespace view
}  // namespace gal

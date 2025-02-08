#pragma once

#include <algorithm>
#include <array>
#include <numeric>

#include <Context.h>
#include <GLUtil.h>
#include <Mesh.h>

namespace gal {
namespace view {

template<>
struct Drawable<TriMesh> : public std::true_type
{
  static constexpr glm::vec4 sFaceColor      = {1.f, 1.f, 1.f, 1.f};
  static constexpr glm::vec4 sWireframeColor = {.8f, .8f, .8f, 1.f};
  static constexpr glm::vec4 sEdgeColor      = {0.f, 0.f, 0.f, 1.f};
  using MeshT                                = SafeInstanceType<TriMesh>;

private:
  glutil::MeshVertexBuffer mVBuf;
  glutil::IndexBuffer      mIBuf;
  Box3                     mBounds;

public:
  void update(const std::vector<MeshT>& meshes)
  {
    mBounds = gal::Box3();
    mVBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const MeshT& mesh) {
        return total + mesh->n_vertices();
      }));
    mIBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const MeshT& mesh) {
        return total + 3 * mesh->n_faces();
      }));

    auto      vbegin = mVBuf.begin();
    uint32_t* dsti   = mIBuf.data();
    uint32_t  offset = 0;
    for (const auto& meshptr : meshes) {
      meshptr->update_normals();
      const auto& mesh = *meshptr;
      if (std::all_of(mesh.vertices_begin(), mesh.vertices_end(), [&mesh](VertH v) {
            return mesh.color(v) == glm::vec3 {0.f, 0.f, 0.f};
          })) {
        for (VertH v : mesh.vertices()) {
          *(vbegin++) = {mesh.point(v), mesh.normal(v), {1.f, 1.f, 1.f}};
        }
      }
      else {
        for (VertH v : mesh.vertices()) {
          *(vbegin++) = {mesh.point(v), mesh.normal(v), mesh.color(v)};
        }
      }
      // 3 indices per face and nothing else.
      for (FaceH f : mesh.faces()) {
        std::transform(mesh.cfv_begin(f), mesh.cfv_end(f), dsti, [&](VertH v) {
          return offset + uint32_t(v.idx());
        });
        dsti += 3;
      }
      offset += uint32_t(mesh.n_vertices());

      mBounds.inflate(mesh.bounds());
    }
    mVBuf.alloc();
    mIBuf.alloc();
    std::cout << "Vertex buffer size: " << mVBuf.size() << std::endl;
    std::cout << "Index buffer size: " << mIBuf.size() << std::endl;
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
    static RenderSettings settings = renderSettings();
    std::cout << "Drawing mesh...\n";
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

template<>
struct Drawable<PolyMesh> : public std::true_type
{
  static constexpr glm::vec4 sFaceColor      = {1.f, 1.f, 1.f, 1.f};
  static constexpr glm::vec4 sWireframeColor = {.8f, .8f, .8f, 1.f};
  static constexpr glm::vec4 sLineColor      = {1.f, 1.f, 1.f, 1.f};
  static constexpr glm::vec4 sEdgeColor      = {0.f, 0.f, 0.f, 1.f};
  using MeshT                                = SafeInstanceType<PolyMesh>;

private:
  glutil::MeshVertexBuffer mVBuf;
  glutil::IndexBuffer      mIBuf;
  glutil::VertexBuffer     mEBuf;
  Box3                     mBounds;

public:
  void update(const std::vector<MeshT>& meshes)
  {
    mBounds = gal::Box3();
    mVBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const MeshT& meshptr) {
        return total + meshptr->n_vertices();
      }));
    mIBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const MeshT& meshptr) {
        size_t nTriangles = 0;
        for (auto fh : meshptr->faces()) {
          nTriangles +=
            size_t(std::distance(meshptr->cfv_begin(fh), meshptr->cfv_end(fh))) - 2;
        }
        return total + 3 * nTriangles;
      }));
    mEBuf.resize(std::accumulate(
      meshes.begin(), meshes.end(), size_t(0), [](size_t total, const MeshT& meshptr) {
        return total + 2 * meshptr->n_edges();
      }));
    auto     vdst   = mVBuf.begin();
    auto     idst   = mIBuf.begin();
    auto     edst   = mEBuf.begin();
    uint32_t offset = 0;
    for (const auto& meshptr : meshes) {
      meshptr->update_normals();
      const auto& mesh = *meshptr;
      if (std::all_of(mesh.vertices_begin(), mesh.vertices_end(), [&mesh](VertH v) {
            return mesh.color(v) == glm::vec3 {0.f, 0.f, 0.f};
          })) {
        for (VertH v : mesh.vertices()) {
          *(vdst++) = {mesh.point(v), mesh.normal(v), {1.f, 1.f, 1.f}};
        }
      }
      else {
        for (auto vh : mesh.vertices()) {
          *(vdst++) = {mesh.point(vh), mesh.normal(vh), mesh.color(vh)};
        }
      }
      for (auto fh : mesh.faces()) {
        auto     fv2  = mesh.cfv_begin(fh);
        uint32_t v0   = offset + uint32_t((fv2++)->idx());
        auto     fv1  = fv2++;
        auto     fend = mesh.cfv_end(fh);
        while (fv2 != fend) {
          *(idst++) = v0;
          *(idst++) = offset + uint32_t((fv1++)->idx());
          *(idst++) = offset + uint32_t((fv2++)->idx());
        }
      }
      offset += uint32_t(mesh.n_vertices());
      for (auto eh : mesh.edges()) {
        auto heh  = handle<HalfH>(mesh.halfedge_handle(eh, 0));
        *(edst++) = {mesh.point(mesh.from_vertex_handle(heh)), glm::vec3(0.f)};
        *(edst++) = {mesh.point(mesh.to_vertex_handle(heh)), glm::vec3(0.f)};
      }
      mBounds.inflate(mesh.bounds());
    }
    mVBuf.alloc();
    mIBuf.alloc();
    mEBuf.alloc();
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
    static RenderSettings settings = renderSettings();
    settings.edgeMode              = false;
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
      settings.edgeColor     = sEdgeColor;
      settings.faceColor     = sEdgeColor;
      settings.shadingFactor = 0.f;
      settings.apply();
      mEBuf.bindVao();
      mEBuf.bindVbo();
      GL_CALL(glDisable(GL_POLYGON_OFFSET_FILL));
      GL_CALL(glDrawArrays(GL_LINES, 0, mEBuf.size()));
    }
  }
};

}  // namespace view
}  // namespace gal

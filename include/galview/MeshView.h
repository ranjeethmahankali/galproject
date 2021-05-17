#pragma once

#include <galcore/Mesh.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <array>

namespace gal {
namespace view {

class MeshView : public Drawable
{
  friend struct MakeDrawable<gal::Mesh>;

public:
  MeshView() = default;
  ~MeshView();

  void draw() const;

private:
  MeshView(const MeshView&) = delete;
  const MeshView& operator=(const MeshView&) = delete;

  MeshView(MeshView&&) = default;

  void drawInternal() const;

private:
  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.a
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.
};

template<>
struct MakeDrawable<gal::Mesh> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const gal::Mesh&             mesh,
                                       std::vector<RenderSettings>& renderSettings)
  {
    std::shared_ptr<MeshView> view = std::make_shared<MeshView>();

    // Position and Normal for each vertex.
    glutil::VertexBuffer vBuf(mesh.numVertices());
    auto                 vbegin = vBuf.begin();
    size_t               nVerts = mesh.numVertices();
    for (size_t i = 0; i < nVerts; i++) {
      *(vbegin++) = {mesh.vertex(i), mesh.vertexNormal(i)};
    }
    view->mVSize = (uint32_t)vBuf.size();
    view->setBounds(mesh.bounds());

    // 3 indices per face and nothing else.
    glutil::IndexBuffer iBuf(3 * mesh.numFaces());
    uint32_t*           dsti   = iBuf.data();
    auto                fbegin = mesh.faceCBegin();
    auto                fend   = mesh.faceCEnd();
    while (fbegin != fend) {
      const Mesh::Face& face = *(fbegin++);
      *(dsti++)              = (uint32_t)face.a;
      *(dsti++)              = (uint32_t)face.b;
      *(dsti++)              = (uint32_t)face.c;
    }
    view->mISize = (uint32_t)iBuf.size();

    vBuf.finalize(view->mVAO, view->mVBO);
    iBuf.finalize(view->mIBO);

    // Render settings
    static constexpr glm::vec4 sFaceColor = {1.0, 1.0, 1.0, 1.0};
    static constexpr glm::vec4 sEdgeColor = {0.0, 0.0, 0.0, 1.0};
    RenderSettings             settings;
    settings.faceColor   = sFaceColor;
    settings.edgeColor   = sEdgeColor;
    settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    renderSettings.push_back(settings);
    if (Context::get().wireframeMode()) {
      settings.edgeColor   = {0.f, 0.f, 0.f, 1.f};
      settings.faceColor   = {0.f, 0.f, 0.f, 1.f};
      settings.polygonMode = std::make_pair(GL_FRONT_AND_BACK, GL_LINE);
      renderSettings.push_back(settings);
    }
    return view;
  };
};

}  // namespace view
}  // namespace gal

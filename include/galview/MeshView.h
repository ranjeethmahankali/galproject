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
struct MakeDrawable<gal::Mesh>
{
  static std::shared_ptr<Drawable> get(const gal::Mesh&             mesh,
                                       std::vector<RenderSettings>& renderSettings)
  {
    std::shared_ptr<MeshView> view = std::make_shared<MeshView>();

    // 3 Coords per vertex and nothing else.
    size_t nVerts = mesh.numVertices();
    // Position and Normal for each vertex.
    std::vector<float> vBuf(6 * mesh.numVertices(), FLT_MAX);
    float*             dstf = vBuf.data();
    for (size_t i = 0; i < nVerts; i++) {
      utils::copy_coords(mesh.vertex(i), dstf);
      utils::copy_coords(mesh.vertexNormal(i), dstf);
    }
    view->mVSize = sizeof(float) * vBuf.size();

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
    view->mISize = (uint32_t)iBuf.size();

    // Now write the data to the device.
    GL_CALL(glGenVertexArrays(1, &view->mVAO));
    GL_CALL(glGenBuffers(1, &view->mVBO));
    GL_CALL(glGenBuffers(1, &view->mIBO));

    GL_CALL(glBindVertexArray(view->mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, view->mVBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, view->mVSize, vBuf.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, view->mIBO));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         sizeof(uint32_t) * iBuf.size(),
                         iBuf.data(),
                         GL_STATIC_DRAW));

    // Vertex position attribute.
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CALL(glEnableVertexAttribArray(1));

    // Unbind stuff.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));

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

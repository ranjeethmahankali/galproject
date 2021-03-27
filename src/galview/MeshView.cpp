#include <galview/Context.h>
#include <galview/MeshView.h>
#include <stdint.h>

namespace gal {
namespace view {

MeshView::~MeshView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
  GL_CALL(glDeleteBuffers(1, &mIBO));
}

void MeshView::drawInternal() const
{
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));
}

void MeshView::draw() const
{
  static constexpr glm::vec4 sFaceColor = {1.0, 1.0, 1.0, 1.0};
  static constexpr glm::vec4 sEdgeColor = {0.0, 0.0, 0.0, 1.0};
  Context::get().setUniform<glm::vec4>("faceColor", sFaceColor);
  Context::get().setUniform<glm::vec4>("edgeColor", sEdgeColor);
  Context::get().setUniform<bool>("edgeMode", false);
  Context::get().setUniform<bool>("pointMode", false);
  Context::get().setUniform<float>("shadingFactor", 1.0f);
  GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  drawInternal();
  if (Context::get().wireframeMode()) {
    Context::get(). setUniform<bool>("edgeMode", true);
    GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    drawInternal();
  }
};

}  // namespace view
}  // namespace gal
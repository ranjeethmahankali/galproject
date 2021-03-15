#include <galview/SphereView.h>

namespace gal {
namespace view {

SphereView::~SphereView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
  GL_CALL(glDeleteBuffers(1, &mIBO));
};

void SphereView::draw() const
{
  static constexpr glm::vec4 sFaceColor = {1.0f, 1.0f, 1.0f, 1.0f};
  static constexpr glm::vec4 sEdgeColor = {0.0f, 0.0f, 0.0f, 1.0f};
  Context::get().setUniform<glm::vec4>("faceColor", sFaceColor);
  Context::get().setUniform<glm::vec4>("edgeColor", sEdgeColor);
  Context::get().setUniform<float>("shadingFactor", 0.9f);
  Context::get().setUniform<bool>("edgeMode", false);
  GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));
};

}  // namespace view
}  // namespace gal
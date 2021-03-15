#include <galview/BoxView.h>

namespace gal {
namespace view {

BoxView::~BoxView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void BoxView::draw() const {
  static constexpr glm::vec4 sLineColor = {1.0, 1.0, 1.0, 1.0};
  Context::get().setUniform<glm::vec4>("faceColor", sLineColor);
  Context::get().setUniform<glm::vec4>("edgeColor", sLineColor);
  Context::get().setUniform<float>("shadingFactor", 0.0f);
  Context::get().setUniform<bool>("edgeMode", false);
  Context::get().setUniform<bool>("pointMode", false);
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_LINES, mISize, GL_UNSIGNED_INT, nullptr));
}

}  // namespace view
}  // namespace gal
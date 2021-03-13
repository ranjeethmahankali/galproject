#include <galview/PlaneView.h>

namespace gal {
namespace view {

PlaneView::~PlaneView() {
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void PlaneView::draw() const {
  static constexpr glm::vec4 sFaceColor = {0.5, 0.5, 0.9, 0.3};
  Context::get().setUniform<glm::vec4>("faceColor", sFaceColor);
  Context::get().setUniform<bool>("edgeMode", false);
  Context::get().setUniform<float>("shadingFactor", 0.0f);
  GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVSize));
}

}
}  // namespace gal
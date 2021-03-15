#include <galview/PointCloudView.h>

namespace gal {
namespace view {

PointCloudView::~PointCloudView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void PointCloudView::draw() const
{
  static constexpr glm::vec4 sPointColor = {0.0, 0.7, 0.0, 1.0};
  Context::get().setUniform<glm::vec4>("pointColor", sPointColor);
  Context::get().setUniform<bool>("edgeMode", false);
  Context::get().setUniform<bool>("pointMode", true);
  Context::get().setUniform<float>("shadingFactor", 1.0f);

  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_POINTS, 0, mVSize));
//   GL_CALL(glDrawArrays(GL_LINE_STRIP, 0, mVSize));
};

}  // namespace view
}  // namespace gal
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
  GL_CALL(glEnable(GL_CULL_FACE));
  GL_CALL(glCullFace(GL_BACK));
  GL_CALL(glFrontFace(GL_CW));

  GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));

  GL_CALL(glDisable(GL_CULL_FACE));
};

}  // namespace view
}  // namespace gal
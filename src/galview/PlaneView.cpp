#include <galview/PlaneView.h>

namespace gal {
namespace view {

PlaneView::~PlaneView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void PlaneView::draw() const
{
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, mVSize));
};

bool PlaneView::opaque() const
{
  return false;
}

}  // namespace view
}  // namespace gal

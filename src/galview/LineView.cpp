#include <galview/LineView.h>

namespace gal {
namespace view {

LineView::~LineView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
}

void LineView::draw() const
{
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_LINES, 0, mVSize));
}

}  // namespace view
}  // namespace gal
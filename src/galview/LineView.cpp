#include <galview/LineView.h>

namespace gal {
namespace view {

Line2dView::~Line2dView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
}

void Line2dView::draw() const
{
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_LINE, 0, mVSize));
}

}  // namespace view
}  // namespace gal
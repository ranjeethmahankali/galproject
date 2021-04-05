#include <galview/BoxView.h>

namespace gal {
namespace view {

BoxView::~BoxView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void BoxView::draw() const {
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_LINES, mISize, GL_UNSIGNED_INT, nullptr));
}

}  // namespace view
}  // namespace gal
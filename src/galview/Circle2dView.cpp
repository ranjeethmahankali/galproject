#include <galview/Circle2dView.h>

namespace gal {
namespace view {

Circle2dView::~Circle2dView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void Circle2dView::draw() const
{
  static const size_t shaderId = Context::get().shaderId("default");
  Context::get().useShader(shaderId);
  
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_LINE_LOOP, 0, mVSize));
};

}  // namespace view
}  // namespace gal
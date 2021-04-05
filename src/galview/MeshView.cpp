#include <galview/Context.h>
#include <galview/MeshView.h>
#include <stdint.h>

namespace gal {
namespace view {

MeshView::~MeshView()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
  GL_CALL(glDeleteBuffers(1, &mIBO));
}

void MeshView::draw() const
{
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
  GL_CALL(glDrawElements(GL_TRIANGLES, mISize, GL_UNSIGNED_INT, nullptr));
};

}  // namespace view
}  // namespace gal
#pragma Once
#include <galcore/Plane.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>

namespace gal {
namespace view {

template<>
struct Drawable<Plane> : std::true_type
{
private:
  Box3 mBounds;
  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mVSize = 0;  // vertex buffer size.

public:
  Drawable<Plane>(const Plane& plane)
  {
    static constexpr float sHalfSize = 2.0f;
    glm::vec3              x = plane.xaxis(), y = plane.yaxis();

    glutil::VertexBuffer vBuf(4);
    auto                 vbegin = vBuf.begin();
    *(vbegin++) = {plane.origin() - (x * sHalfSize) - (y * sHalfSize), plane.normal()};
    *(vbegin++) = {plane.origin() + (x * sHalfSize) - (y * sHalfSize), plane.normal()};
    *(vbegin++) = {plane.origin() - (x * sHalfSize) + (y * sHalfSize), plane.normal()};
    *(vbegin++) = {plane.origin() + (x * sHalfSize) + (y * sHalfSize), plane.normal()};
    mVSize      = vBuf.size();

    vbegin = vBuf.begin();
    while (vbegin != vBuf.end()) {
      mBounds.inflate(vbegin->position);
      vbegin++;
    }

    vBuf.finalize(mVAO, mVBO);
  }

  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;

  const Drawable& operator=(Drawable&& other)
  {
    mVAO = std::exchange(other.mVAO, 0);
    mVBO = std::exchange(other.mVBO, 0);
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  ~Drawable<Plane>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
  }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sFaceColor = {0.7, 0.0, 0.0, 0.2};
    RenderSettings             settings;
    settings.shaderId      = Context::get().shaderId("default");
    settings.faceColor     = sFaceColor;
    settings.shadingFactor = 0.0f;
    settings.polygonMode   = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    return settings;
  }

  Box3 bounds() const { return mBounds; }

  void draw() const
  {
    static auto rsettings = settings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, mVSize));
  }
};

}  // namespace view
}  // namespace gal

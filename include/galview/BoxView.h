#include <galcore/Box.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<Box3> : public std::true_type
{
private:
  Box3 mBounds;
  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.a
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.

public:
  Drawable<Box3>(const Box3& box)
      : mBounds(box)
  {
    glutil::VertexBuffer vBuf(8);

    auto vbegin = vBuf.begin();
    *(vbegin++) = {{box.min.x, box.min.y, box.min.z}};
    *(vbegin++) = {{box.max.x, box.min.y, box.min.z}};
    *(vbegin++) = {{box.max.x, box.max.y, box.min.z}};
    *(vbegin++) = {{box.min.x, box.max.y, box.min.z}};
    *(vbegin++) = {{box.min.x, box.min.y, box.max.z}};
    *(vbegin++) = {{box.max.x, box.min.y, box.max.z}};
    *(vbegin++) = {{box.max.x, box.max.y, box.max.z}};
    *(vbegin++) = {{box.min.x, box.max.y, box.max.z}};

    static constexpr std::array<uint32_t, 24> sIBuf = {{
      0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
    }};

    glutil::IndexBuffer iBuf(sIBuf.size());
    std::copy(sIBuf.begin(), sIBuf.end(), iBuf.begin());

    mVSize = (uint32_t)vBuf.size();
    mISize = (uint32_t)iBuf.size();
    vBuf.finalize(mVAO, mVBO);
    iBuf.finalize(mIBO);
  }

  ~Drawable<Box3>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
    if (mIBO) {
      GL_CALL(glDeleteBuffers(1, &mIBO));
    }
  }

  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;

  const Drawable& operator=(Drawable&& other)
  {
    mVAO = std::exchange(other.mVAO, 0);
    mVBO = std::exchange(other.mVBO, 0);
    mIBO = std::exchange(other.mIBO, 0);
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  Box3 bounds() const { return mBounds; }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sLineColor = {1.0, 1.0, 1.0, 1.0};
    RenderSettings             settings;
    settings.shaderId      = Context::get().shaderId("default");
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.0f;
    return settings;
  }

  void draw() const
  {
    static auto rsettings = settings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
    GL_CALL(glDrawElements(GL_LINES, mISize, GL_UNSIGNED_INT, nullptr));
  }
};

}  // namespace view
}  // namespace gal

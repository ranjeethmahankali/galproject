#pragma once
#include <galcore/PointCloud.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<PointCloud> : public std::true_type
{
private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<PointCloud>(const PointCloud& cloud)
      : mBounds(cloud.bounds())
  {
    glutil::VertexBuffer vBuf(cloud.size());
    std::transform(
      cloud.cbegin(),
      cloud.cend(),
      vBuf.begin(),
      [](const glm::vec3& pt) -> glutil::VertexBuffer::VertexType { return {pt}; });
    mVSize = vBuf.size();
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<PointCloud>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
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

  Box3 bounds() const { return mBounds; }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};
    RenderSettings             settings;
    settings.shaderId   = Context::get().shaderId("default");
    settings.pointColor = sPointColor;
    settings.pointMode  = true;
    return settings;
  }

  void draw() const
  {
    static auto rsettings = settings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_POINTS, 0, mVSize));
  }
};

}  // namespace view
}  // namespace gal

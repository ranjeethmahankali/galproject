#pragma once
#include <galcore/PointCloud.h>
#include <galview/Context.h>
#include <numeric>

namespace gal {
namespace view {

template<>
struct Drawable<glm::vec3> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<glm::vec3>(const std::vector<glm::vec3>& points)
  {
    glutil::VertexBuffer vBuf(points.size());

    std::transform(
      points.cbegin(),
      points.cend(),
      vBuf.begin(),
      [](const glm::vec3& pt) -> glutil::VertexBuffer::VertexType { return {pt}; });

    for (const auto& pt : points) {
      mBounds.inflate(pt);
    }

    mVSize = vBuf.size();
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<glm::vec3>()
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
    mBounds = other.mBounds;
    mVAO    = std::exchange(other.mVAO, 0);
    mVBO    = std::exchange(other.mVBO, 0);
    mVSize  = other.mVSize;
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  Box3 bounds() const { return mBounds; }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = uint64_t((1.f - sPointColor.a) * 255.f);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.shaderId   = Context::get().shaderId("default");
    settings.pointColor = sPointColor;
    settings.pointMode  = true;
    return settings;
  }

  void draw() const
  {
    static auto rsettings = renderSettings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_POINTS, 0, mVSize));
  }
};

}  // namespace view
}  // namespace gal

#pragma once

#include <galcore/Circle2d.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<Circle2d> : public std::true_type
{
  static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};

private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<Circle2d>(const std::vector<Circle2d>& circles)
  {
    static constexpr size_t sNumPts = 256;
    static constexpr float  sStep   = (2.0f * M_PI) / float(sNumPts);
    glutil::VertexBuffer    vBuf(2 * sNumPts * circles.size());

    auto vbegin = vBuf.begin();
    for (const auto& circle : circles) {
      glm::vec3 center(circle.center().x, circle.center().y, 0.0f);
      float     radius = circle.radius();
      float     ang    = 0.0f;
      for (size_t i = 0; i < sNumPts; i++) {
        *(vbegin++) = {center +
                       glm::vec3(radius * std::cos(ang), radius * std::sin(ang), 0.f)};
        float ang2  = ang + sStep;
        *(vbegin++) = {center +
                       glm::vec3(radius * std::cos(ang2), radius * std::sin(ang2), 0.f)};
        ang += sStep;
      }
      mBounds.inflate(circle.bounds());
    }

    mVSize = vBuf.size();
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<Circle2d>()
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

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = (uint64_t((1.f - sLineColor.a) * 255.f) << 8) |
                                 (uint64_t((1.f - sLineColor.a) * 255.f) << 16);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.shaderId      = Context::get().shaderId("default");
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.f;
    return settings;
  }

  Box3 bounds() const { return mBounds; }

  void draw() const
  {
    static auto rsettings = renderSettings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_LINES, 0, mVSize));
  }
};

}  // namespace view
}  // namespace gal

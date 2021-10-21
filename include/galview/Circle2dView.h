#pragma once

#include <galcore/Circle2d.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<Circle2d> : public std::true_type
{
private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<Circle2d>(const Circle2d& circle)
      : mBounds(circle.bounds())
  {
    static constexpr size_t sNumPts = 256;
    static constexpr float  sStep   = (2.0f * M_PI) / float(sNumPts);
    glutil::VertexBuffer    vBuf(sNumPts);

    glm::vec3 center(circle.center().x, circle.center().y, 0.0f);
    float     radius = circle.radius();
    auto      vbegin = vBuf.begin();
    float     ang    = 0.0f;
    for (size_t i = 0; i < sNumPts; i++) {
      *(vbegin++) = {center +
                     glm::vec3(radius * std::cos(ang), radius * std::sin(ang), 0.f)};
      ang += sStep;
    }
    mVSize = sNumPts;
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
    mVAO = std::exchange(other.mVAO, 0);
    mVBO = std::exchange(other.mVBO, 0);
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};
    RenderSettings             settings;
    settings.shaderId      = Context::get().shaderId("default");
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.f;
    return settings;
  }

  Box3 bounds() const { return mBounds; }

  void draw() const
  {
    static auto rsettings = settings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_LINE_LOOP, 0, mVSize));
  }
};

}  // namespace view
}  // namespace gal

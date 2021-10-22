#pragma once

#include <galcore/Line.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<Line2d> : public std::true_type
{
  static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};

private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<Line2d>(const Line2d& line)
      : mBounds(line.bounds())
  {
    static constexpr glm::vec3 sZero = {0.f, 0.f, 0.f};
    glutil::VertexBuffer       vBuf(2);
    vBuf[0] = {glm::vec3(line.mStart, 0.f), sZero};
    vBuf[1] = {glm::vec3(line.mEnd, 0.f), sZero};
    mVSize  = 2;
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<Line2d>()
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
    static const uint64_t sIdx = uint64_t(0x0000ff) |
                                 (uint64_t((1.f - sLineColor.a) * 255.f) << 8) |
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

  void draw() const
  {
    static auto rsettings = renderSettings();
    rsettings.apply();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_LINES, 0, mVSize));
  }
};

template<>
struct Drawable<Line3d> : public std::true_type
{
  static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};

private:
  Box3     mBounds;
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;

public:
  Drawable<Line3d>(const Line3d& line)
      : mBounds(line.bounds())
  {
    static constexpr glm::vec3 sZero = {0.f, 0.f, 0.f};
    glutil::VertexBuffer       vBuf(2);
    vBuf[0] = {line.mStart, sZero};
    vBuf[1] = {line.mEnd, sZero};
    mVSize  = 2;
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<Line3d>()
  {
    GL_CALL(glDeleteVertexArrays(1, &mVAO));
    GL_CALL(glDeleteBuffers(1, &mVBO));
  }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = uint64_t(0x0000ff) |
                                 (uint64_t((1.f - sLineColor.a) * 255.f) << 8) |
                                 (uint64_t((1.f - sLineColor.a) * 255.f) << 16);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
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

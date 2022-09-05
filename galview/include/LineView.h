#pragma once

#include <Context.h>
#include <Line.h>

namespace gal {
namespace view {

template<>
struct Drawable<Line2d> : public std::true_type
{
  static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};

private:
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;

public:
  void update(const std::vector<Line2d>& lines)
  {
    mBounds = gal::Box3();
    mVBuf.resize(2 * lines.size());
    static constexpr glm::vec3 sZero = {0.f, 0.f, 0.f};

    auto vbegin = mVBuf.begin();
    for (const auto& line : lines) {
      *(vbegin++) = {glm::vec3(line.mStart, 0.f), sZero};
      *(vbegin++) = {glm::vec3(line.mEnd, 0.f), sZero};
      mBounds.inflate(Box3(line.bounds()));
    }
    mVBuf.alloc();
  }

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
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_LINES, 0, mVBuf.size()));
  }
};

template<>
struct Drawable<Line3d> : public std::true_type
{
  static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};

private:
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;

public:
  void update(const std::vector<Line3d>& lines)
  {
    mVBuf.resize(2 * lines.size());
    static constexpr glm::vec3 sZero = {0.f, 0.f, 0.f};

    auto vbegin = mVBuf.begin();
    for (const auto& line : lines) {
      *(vbegin++) = {line.mStart, sZero};
      *(vbegin++) = {line.mEnd, sZero};
      mBounds.inflate(line.bounds());
    }
    mVBuf.alloc();
  }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = (uint64_t((1.f - sLineColor.a) * 255.f) << 8) |
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
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_LINES, 0, mVBuf.size()));
  }
};

}  // namespace view
}  // namespace gal

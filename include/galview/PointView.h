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
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;

public:
  Drawable<glm::vec3>(const std::vector<glm::vec3>& points)
      : mVBuf(points.size())
  {
    std::transform(
      points.cbegin(),
      points.cend(),
      mVBuf.begin(),
      [](const glm::vec3& pt) -> glutil::VertexBuffer::VertexType { return {pt}; });

    for (const auto& pt : points) {
      mBounds.inflate(pt);
    }

    mVBuf.alloc();
  }

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
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_POINTS, 0, mVBuf.size()));
  }
};

template<>
struct Drawable<glm::vec2> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

private:
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;

public:
  Drawable(const std::vector<glm::vec2>& points)
      : mVBuf(points.size())
  {
    std::transform(points.cbegin(),
                   points.cend(),
                   mVBuf.begin(),
                   [](const glm::vec2& pt) -> glutil::VertexBuffer::VertexType {
                     return {glm::vec3(pt, 0.f)};
                   });

    for (const auto& pt : points) {
      mBounds.inflate(glm::vec3(pt, 0.f));
    }

    mVBuf.alloc();
  }

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
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_POINTS, 0, mVBuf.size()));
  }
};  // namespace view

}  // namespace view
}  // namespace gal

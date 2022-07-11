#pragma once
#include <galcore/PointCloud.h>
#include <galview/Context.h>
#include <numeric>

namespace gal {
namespace view {

template<>
struct Drawable<PointCloud<3>> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

private:
  Box3                 mBounds;
  glutil::VertexBuffer mVBuf;

public:
  void update(const std::vector<PointCloud<3>>& clouds)
  {
    mBounds = gal::Box3();
    mVBuf.resize(std::accumulate(
      clouds.begin(),
      clouds.end(),
      size_t(0),
      [](size_t total, const PointCloud<3>& cloud) { return total + cloud.size(); }));
    for (const auto& cloud : clouds) {
      std::transform(
        cloud.cbegin(),
        cloud.cend(),
        mVBuf.begin(),
        [](const glm::vec3& pt) -> glutil::VertexBuffer::VertexType { return {pt}; });
      mBounds.inflate(cloud.bounds());
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

}  // namespace view
}  // namespace gal

#pragma once

#include <numeric>

#include <Context.h>
#include <PointCloud.h>

namespace gal {
namespace view {

template<int N, typename T, glm::qualifier Q>
struct Drawable<glm::vec<N, T, Q>> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

private:
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;

  static glm::vec3 toVec3(const glm::vec<N, T, Q>& pt)
  {
    static constexpr int sNumCoords = std::min(N, 3);
    glm::vec3            pos(0.f);
    for (int i = 0; i < sNumCoords; i++) {
      pos[i] = float(pt[i]);
    }
    return pos;
  }

public:
  void update(const std::vector<glm::vec<N, T, Q>>& points)
  {
    mBounds = gal::Box3();
    mVBuf.resize(points.size());
    std::transform(
      points.cbegin(),
      points.cend(),
      mVBuf.begin(),
      [this](const glm::vec<N, T, Q>& pt) -> glutil::VertexBuffer::VertexType {
        auto pos = toVec3(pt);
        mBounds.inflate(pos);
        return {toVec3(pt)};
      });

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

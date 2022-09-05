#pragma once

#include <Context.h>
#include <GLUtil.h>
#include <Plane.h>

namespace gal {
namespace view {

template<>
struct Drawable<Plane> : std::true_type
{
  static constexpr glm::vec4 sFaceColor = {0.7, 0.0, 0.0, 0.2};

private:
  glutil::VertexBuffer mVBuf;
  Box3                 mBounds;
  uint                 mVAO   = 0;  // vertex array object.
  uint                 mVBO   = 0;  // vertex buffer object.
  uint                 mVSize = 0;  // vertex buffer size.

public:
  void update(const std::vector<Plane>& planes)
  {
    mBounds = gal::Box3();
    mVBuf.resize(6 * planes.size());
    static constexpr float sHalfSize = 2.0f;
    auto                   vbegin    = mVBuf.begin();

    for (const auto& plane : planes) {
      glm::vec3             x = plane.xaxis(), y = plane.yaxis();
      glutil::DefaultVertex a = {plane.origin() - (x * sHalfSize) - (y * sHalfSize),
                                 plane.normal()};
      glutil::DefaultVertex b = {plane.origin() + (x * sHalfSize) + (y * sHalfSize),
                                 plane.normal()};

      *(vbegin++) = a;
      *(vbegin++) = {plane.origin() + (x * sHalfSize) - (y * sHalfSize), plane.normal()};
      *(vbegin++) = b;

      *(vbegin++) = a;
      *(vbegin++) = b;
      *(vbegin++) = {plane.origin() - (x * sHalfSize) + (y * sHalfSize), plane.normal()};
    }

    for (const auto& v : mVBuf) {
      mBounds.inflate(v.position);
    }

    mVBuf.alloc();
  }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = (uint64_t((1.f - sFaceColor.a) * 255.f) << 16);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.shaderId      = Context::get().shaderId("default");
    settings.faceColor     = sFaceColor;
    settings.shadingFactor = 0.0f;
    settings.polygonMode   = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    return settings;
  }

  Box3 bounds() const { return mBounds; }

  void draw() const
  {
    static auto rsettings = renderSettings();
    rsettings.apply();
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
  }
};

}  // namespace view
}  // namespace gal

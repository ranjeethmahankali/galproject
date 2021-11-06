#include <galcore/Box.h>
#include <galview/Context.h>

namespace gal {
namespace view {

template<>
struct Drawable<Box3> : public std::true_type
{
  static constexpr glm::vec4                sLineColor = {1.0, 1.0, 1.0, 1.0};
  static constexpr std::array<uint32_t, 24> sIBuf      = {{
    0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
  }};

private:
  glutil::VertexBuffer mVBuf;
  glutil::IndexBuffer  mIBuf;
  Box3                 mBounds;

public:
  void update(const std::vector<Box3>& boxes)
  {
    mBounds = gal::Box3();
    mVBuf.resize(8 * boxes.size());
    mIBuf.resize(sIBuf.size() * boxes.size());
    auto   vbegin = mVBuf.begin();
    auto   ibegin = mIBuf.begin();
    size_t off    = 0;
    for (const auto& box : boxes) {
      *(vbegin++) = {{box.min.x, box.min.y, box.min.z}};
      *(vbegin++) = {{box.max.x, box.min.y, box.min.z}};
      *(vbegin++) = {{box.max.x, box.max.y, box.min.z}};
      *(vbegin++) = {{box.min.x, box.max.y, box.min.z}};
      *(vbegin++) = {{box.min.x, box.min.y, box.max.z}};
      *(vbegin++) = {{box.max.x, box.min.y, box.max.z}};
      *(vbegin++) = {{box.max.x, box.max.y, box.max.z}};
      *(vbegin++) = {{box.min.x, box.max.y, box.max.z}};
      for (auto i : sIBuf) {
        *(ibegin++) = off + i;
      }
      off += 8;
      mBounds.inflate(box);
    }

    mVBuf.alloc();
    mIBuf.alloc();
  }

  Box3 bounds() const { return mBounds; }

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
    settings.shadingFactor = 0.0f;
    return settings;
  }

  void draw() const
  {
    static auto rsettings = renderSettings();
    rsettings.apply();
    mVBuf.bindVao();
    mIBuf.bind();
    GL_CALL(glDrawElements(GL_LINES, mIBuf.size(), GL_UNSIGNED_INT, nullptr));
  }
};

}  // namespace view
}  // namespace gal

#include <galcore/Box.h>
#include <galview/Context.h>

namespace gal {
namespace view {

class BoxView : public Drawable
{
  friend struct MakeDrawable<gal::Box3>;

public:
  BoxView() = default;
  ~BoxView();

  void draw() const override;

private:
  BoxView(const BoxView&) = delete;
  const BoxView& operator=(const BoxView&) = delete;

  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.a
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.
};

template<>
struct MakeDrawable<gal::Box3> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const gal::Box3&             box,
                                       std::vector<RenderSettings>& renderSettings)
  {
    glutil::VertexBuffer vBuf(8);

    auto vbegin = vBuf.begin();
    *(vbegin++) = {{box.min.x, box.min.y, box.min.z}};
    *(vbegin++) = {{box.max.x, box.min.y, box.min.z}};
    *(vbegin++) = {{box.max.x, box.max.y, box.min.z}};
    *(vbegin++) = {{box.min.x, box.max.y, box.min.z}};
    *(vbegin++) = {{box.min.x, box.min.y, box.max.z}};
    *(vbegin++) = {{box.max.x, box.min.y, box.max.z}};
    *(vbegin++) = {{box.max.x, box.max.y, box.max.z}};
    *(vbegin++) = {{box.min.x, box.max.y, box.max.z}};

    static constexpr std::array<uint32_t, 24> sIBuf = {{
      0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
    }};

    glutil::IndexBuffer iBuf(sIBuf.size());
    std::copy(sIBuf.begin(), sIBuf.end(), iBuf.begin());

    auto view = std::make_shared<BoxView>();
    view->setBounds(box);
    view->mVSize = (uint32_t)vBuf.size();
    view->mISize = (uint32_t)iBuf.size();
    vBuf.finalize(view->mVAO, view->mVBO);
    iBuf.finalize(view->mIBO);

    // Render settings.
    static constexpr glm::vec4 sLineColor = {1.0, 1.0, 1.0, 1.0};
    RenderSettings             settings;
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.0f;
    renderSettings.push_back(settings);
    return view;
  }
};

}  // namespace view
}  // namespace gal
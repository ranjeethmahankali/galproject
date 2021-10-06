#pragma Once
#include <galcore/Plane.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>

namespace gal {
namespace view {

class PlaneView : public Drawable
{
  friend struct MakeDrawable<gal::Plane>;

public:
  PlaneView() = default;
  ~PlaneView();

  void draw() const override;

  bool opaque() const override;

private:
  PlaneView(const PlaneView&) = delete;
  const PlaneView& operator=(const PlaneView&) = delete;

  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mVSize = 0;  // vertex buffer size.
};

template<>
struct MakeDrawable<gal::Plane> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const gal::Plane&            plane,
                                       std::vector<RenderSettings>& renderSettings)
  {
    auto view = std::make_shared<gal::view::PlaneView>();

    static constexpr float sHalfSize = 2.0f;
    glm::vec3              x = plane.xaxis(), y = plane.yaxis();

    glutil::VertexBuffer vBuf(4);
    auto                 vbegin = vBuf.begin();
    *(vbegin++)  = {plane.origin() - (x * sHalfSize) - (y * sHalfSize), plane.normal()};
    *(vbegin++)  = {plane.origin() + (x * sHalfSize) - (y * sHalfSize), plane.normal()};
    *(vbegin++)  = {plane.origin() - (x * sHalfSize) + (y * sHalfSize), plane.normal()};
    *(vbegin++)  = {plane.origin() + (x * sHalfSize) + (y * sHalfSize), plane.normal()};
    view->mVSize = vBuf.size();

    Box3 bounds;
    vbegin = vBuf.begin();
    while (vbegin != vBuf.end()) {
      bounds.inflate(vbegin->position);
      vbegin++;
    }
    view->setBounds(bounds);

    vBuf.finalize(view->mVAO, view->mVBO);

    // Render settings.
    static constexpr glm::vec4 sFaceColor = {0.7, 0.0, 0.0, 0.2};
    RenderSettings             settings;
    settings.faceColor     = sFaceColor;
    settings.shadingFactor = 0.0f;
    settings.polygonMode   = std::make_pair(GL_FRONT_AND_BACK, GL_FILL);
    renderSettings.push_back(settings);
    return view;
  };
};

}  // namespace view
}  // namespace gal

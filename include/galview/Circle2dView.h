#pragma once

#include <galcore/Circle2d.h>
#include <galview/Context.h>

namespace gal {
namespace view {

class Circle2dView : public Drawable
{
  friend struct MakeDrawable<Circle2d>;

public:
  Circle2dView() = default;
  ~Circle2dView();

  void draw() const;

private:
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;
};

template<>
struct MakeDrawable<Circle2d> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const Circle2d&              circle,
                                       std::vector<RenderSettings>& renderSettings)
  {
    static constexpr size_t sNumPts = 256;
    static constexpr float  sStep   = (2.0f * M_PI) / float(sNumPts);
    glutil::VertexBuffer    vBuf(sNumPts);

    glm::vec3 center(circle.center().x, circle.center().y, 0.0f);
    float     radius = circle.radius();

    auto  vbegin = vBuf.begin();
    float ang    = 0.0f;
    for (size_t i = 0; i < sNumPts; i++) {
      *(vbegin++) = {center +
                     glm::vec3(radius * std::cos(ang), radius * std::sin(ang), 0.f)};
      ang += sStep;
    }

    auto view = std::make_shared<Circle2dView>();
    view->setBounds(Box3(circle.bounds()));
    view->mVSize = sNumPts;

    vBuf.finalize(view->mVAO, view->mVBO);

    // Render settings.
    static constexpr glm::vec4 sLineColor = {1.0, 1.0, 1.0, 1.0};
    RenderSettings             settings;
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.f;
    renderSettings.push_back(settings);

    return view;
  };
};

}  // namespace view
}  // namespace gal
#pragma once

#include <galcore/Line.h>
#include <galview/Context.h>

namespace gal {
namespace view {

class LineView : public Drawable
{
  friend struct MakeDrawable<Line3d>;

public:
  LineView() = default;
  ~LineView();

  void draw() const;

private:
  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;
};

template<>
struct MakeDrawable<Line3d> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const Line3d&                line,
                                       std::vector<RenderSettings>& renderSettings)
  {
    static constexpr glm::vec3 sZero = {0.f, 0.f, 0.f};
    glutil::VertexBuffer       vBuf(2);
    vBuf[0] = {line.mStart, sZero};
    vBuf[1] = {line.mEnd, sZero};

    auto view = std::make_shared<LineView>();
    view->setBounds(Box3(line.bounds()));
    view->mVSize = 2;
    vBuf.finalize(view->mVAO, view->mVBO);

    // Render settings.
    static constexpr glm::vec4 sLineColor = {1.f, 1.f, 1.f, 1.f};
    RenderSettings             settings;
    settings.faceColor     = sLineColor;
    settings.edgeColor     = sLineColor;
    settings.shadingFactor = 0.f;
    renderSettings.push_back(settings);

    return view;
  }
};

template<>
struct MakeDrawable<Line2d> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const Line2d&                line,
                                       std::vector<RenderSettings>& renderSettings)
  {
    return MakeDrawable<Line3d>::get(Line3d {glm::vec3(line.mStart.x, line.mStart.y, 0.f),
                                             glm::vec3(line.mEnd.x, line.mEnd.y, 0.f)},
                                     renderSettings);
  }
};

}  // namespace view
}  // namespace gal

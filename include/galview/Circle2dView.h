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
    std::vector<glm::vec3>  vBuf;
    vBuf.reserve(sNumPts * 2 + 2);

    glm::vec3 center(circle.center().x, circle.center().y, 0.0f);
    float     radius = circle.radius();

    auto inserter = std::back_inserter(vBuf);

    float ang = 0.0f;
    for (size_t i = 0; i < sNumPts; i++) {
      *(inserter++) =
        center + glm::vec3(radius * std::cos(ang), radius * std::sin(ang), 0.f);
      *(inserter++) = glm::vec3(0.f, 0.f, 0.f);
      ang += sStep;
    }

    size_t nvBytes = sizeof(decltype(vBuf)::value_type) * vBuf.size();

    auto view    = std::make_shared<Circle2dView>();
    view->setBounds(Box3(circle.bounds()));
    view->mVSize = sNumPts;

    GL_CALL(glGenVertexArrays(1, &view->mVAO));
    GL_CALL(glGenBuffers(1, &view->mVBO));

    GL_CALL(glBindVertexArray(view->mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, view->mVBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, nvBytes, vBuf.data(), GL_STATIC_DRAW));

    // Vertex position attribute.
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CALL(glEnableVertexAttribArray(1));

    // Unbind stuff.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));

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
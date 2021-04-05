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
struct MakeDrawable<gal::Box3>
{
  static std::shared_ptr<Drawable> get(const gal::Box3&             box,
                                       std::vector<RenderSettings>& renderSettings)
  {
    glm::vec3                  min   = box.min;
    glm::vec3                  max   = box.max;
    static constexpr glm::vec3 sZero = {0.0f, 0.0f, 0.0f};

    std::array<glm::vec3, 16> vBuf = {{
      {min.x, min.y, min.z},
      sZero,
      {max.x, min.y, min.z},
      sZero,
      {max.x, max.y, min.z},
      sZero,
      {min.x, max.y, min.z},
      sZero,
      {min.x, min.y, max.z},
      sZero,
      {max.x, min.y, max.z},
      sZero,
      {max.x, max.y, max.z},
      sZero,
      {min.x, max.y, max.z},
      sZero,
    }};

    static constexpr std::array<uint32_t, 24> iBuf = {{
      0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
    }};

    auto view = std::make_shared<BoxView>();

    view->mVSize = (uint32_t)sizeof(vBuf);
    view->mISize = (uint32_t)iBuf.size();

    // Now write the data to the device.
    GL_CALL(glGenVertexArrays(1, &view->mVAO));
    GL_CALL(glGenBuffers(1, &view->mVBO));
    GL_CALL(glGenBuffers(1, &view->mIBO));

    GL_CALL(glBindVertexArray(view->mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, view->mVBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, view->mVSize, vBuf.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, view->mIBO));
    GL_CALL(
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iBuf), iBuf.data(), GL_STATIC_DRAW));

    // Vertex position attribute.
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr));
    GL_CALL(glEnableVertexAttribArray(0));

    // Unbind stuff.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));

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
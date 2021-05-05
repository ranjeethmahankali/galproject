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

  void draw() const;

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

    static constexpr size_t    sNumVerts = 6;
    static constexpr float     sHalfSize = 2.0f;
    static constexpr glm::vec3 sXAxis    = {1.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 sYAxis    = {0.0f, 1.0f, 0.0f};
    static constexpr glm::vec3 sZAxis    = {0.0f, 0.0f, 1.0f};

    glm::vec3 x = plane.xaxis(), y = plane.yaxis();

    std::array<glm::vec3, 8> vBuf = {{
      plane.origin() - (x * sHalfSize) - (y * sHalfSize),
      plane.normal(),
      plane.origin() + (x * sHalfSize) - (y * sHalfSize),
      plane.normal(),
      plane.origin() - (x * sHalfSize) + (y * sHalfSize),
      plane.normal(),
      plane.origin() + (x * sHalfSize) + (y * sHalfSize),
      plane.normal(),
    }};

    size_t nvBytes = sizeof(vBuf);

    view->mVSize = 4;

    Box3 bounds;
    auto vbegin = vBuf.begin();
    while (vbegin != vBuf.end()) {
        bounds.inflate(*(vbegin++));
        vbegin++;
    }
    view->setBounds(bounds);

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
#pragma Once
#include <galcore/Plane.h>
#include <galview/Context.h>

namespace gal {
namespace view {

class PlaneView : public Drawable
{
  friend struct MakeDrawable<gal::Plane>;

public:
  PlaneView() = default;
  ~PlaneView();

  void draw() const;

private:
  PlaneView(const PlaneView&) = delete;
  const PlaneView& operator=(const PlaneView&) = delete;

  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mVSize = 0;  // vertex buffer size.
};

template<>
struct MakeDrawable<gal::Plane>
{
  static std::shared_ptr<Drawable> get(const gal::Plane& plane)
  {
    auto view = std::make_shared<gal::view::PlaneView>();

    static constexpr size_t    sNumVerts = 6;
    static constexpr float     sHalfSize = 5.0f;
    static constexpr glm::vec3 sXAxis    = {1.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 sYAxis    = {0.0f, 1.0f, 0.0f};
    static constexpr glm::vec3 sZAxis    = {0.0f, 0.0f, 1.0f};

    glm::vec3 x, y;
    if (sZAxis == plane.normal) {
      x = sXAxis;
      y = sYAxis;
    }
    else if (sZAxis == -plane.normal) {
      x = -sXAxis;
      y = -sYAxis;
    }
    else {
      x = glm::cross(plane.normal, sZAxis);
      y = glm::cross(plane.normal, x);
    }

    std::array<glm::vec3, 4> verts = {{
      plane.origin - (x * sHalfSize) - (y * sHalfSize),
      plane.origin + (x * sHalfSize) - (y * sHalfSize),
      plane.origin + (x * sHalfSize) + (y * sHalfSize),
      plane.origin - (x * sHalfSize) + (y * sHalfSize),
    }};

    std::array<glm::vec3, sNumVerts* 2> vBuf = {{
      verts[0],
      plane.normal,
      verts[1],
      plane.normal,
      verts[2],
      plane.normal,
      verts[0],
      plane.normal,
      verts[2],
      plane.normal,
      verts[3],
      plane.normal,
    }};

    view->mVSize = sizeof(vBuf);

    GL_CALL(glGenVertexArrays(1, &view->mVAO));
    GL_CALL(glGenBuffers(1, &view->mVBO));

    GL_CALL(glBindVertexArray(view->mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, view->mVBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, view->mVSize, vBuf.data(), GL_STATIC_DRAW));

    // Vertex position attribute.
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CALL(glEnableVertexAttribArray(1));

    // Unbind stuff.
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    return view;
  };
};

}  // namespace view
}  // namespace gal
#pragma once
#include <galcore/PointCloud.h>
#include <galview/Context.h>

namespace gal {
namespace view {
class PointCloudView : public Drawable
{
  friend struct MakeDrawable<PointCloud>;

public:
  PointCloudView() = default;
  ~PointCloudView();

  void draw() const override;

private:
  PointCloudView(const PointCloudView&) = delete;
  const PointCloudView& operator=(const PointCloudView&) = delete;

  uint32_t mVAO   = 0;
  uint32_t mVBO   = 0;
  uint32_t mVSize = 0;
};

template<>
struct MakeDrawable<PointCloud>
{
  static std::shared_ptr<Drawable> get(const PointCloud& cloud)
  {
    static constexpr glm::vec3 sZero = {0.0f, 0.0f, 0.0f};
    std::vector<glm::vec3>     vBuf(cloud.size() * 2);
    auto                       dstv = vBuf.begin();
    auto                       srcv = cloud.cbegin();
    while (dstv != vBuf.end()) {
      *(dstv++) = *(srcv++);
      *(dstv++) = sZero;
    }

    size_t nvBytes = sizeof(decltype(vBuf)::value_type) * vBuf.size();

    auto view = std::make_shared<PointCloudView>();
    view->mVSize = cloud.size();

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
    return view;
  };
};

}  // namespace view
}  // namespace gal
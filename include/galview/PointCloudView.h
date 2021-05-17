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
struct MakeDrawable<PointCloud> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const PointCloud&            cloud,
                                       std::vector<RenderSettings>& renderSettings)
  {
    glutil::VertexBuffer vBuf(cloud.size());
    std::transform(
      cloud.cbegin(),
      cloud.cend(),
      vBuf.begin(),
      [](const glm::vec3& pt) -> glutil::VertexBuffer::VertexType { return {pt}; });

    auto view    = std::make_shared<PointCloudView>();
    view->mVSize = vBuf.size();
    view->setBounds(cloud.bounds());

    vBuf.finalize(view->mVAO, view->mVBO);

    // Render Settings.
    static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};
    RenderSettings             settings;
    settings.pointColor = sPointColor;
    settings.pointMode  = true;
    renderSettings.push_back(settings);
    return view;
  };
};

}  // namespace view
}  // namespace gal
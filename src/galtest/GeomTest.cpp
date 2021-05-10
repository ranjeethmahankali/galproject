#include <galcore/Circle2d.h>
#include <galcore/DebugProfile.h>
#include <gtest/gtest.h>

TEST(Circle2d, MinBoundingCircle)
{
  GALSCOPE(__func__);
  std::vector<glm::vec2> points {
    {0, 0},
    {1, 0},
    {1, 1},
    {-.3, 1},
    {0, -1},
  };

  auto cloud = gal::PointCloud(points);
  GALCAPTURE(cloud);
  auto circ = gal::Circle2d::minBoundingCircle(points.data(), points.size());

  cloud.emplace_back(.3f, 1.4f, 0.0f);
  GALCAPTURE(cloud);
  for (const auto& pt : points) {
    ASSERT_TRUE(circ.contains(pt));
  }
}
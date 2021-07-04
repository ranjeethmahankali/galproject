#include <galcore/Annotations.h>
#include <galcore/Circle2d.h>
#include <galcore/DebugProfile.h>
#include <gtest/gtest.h>

TEST(Circle2d, MinBoundingCircle)
{
  GALSCOPE(__func__);
  std::vector<glm::vec2> points {
    {0.f, 0.f},
    {1.f, 0.f},
    {0.25f, 0.25f},
    {0.9f, 0.9f},
    {-.3f, 1.f},
    {0.f, -1.f},
  };

  auto cloud = gal::PointCloud(points);
  GALWATCH(cloud);
  GALCAPTURE(gal::Annotations::createIndexedPointCloud(cloud), tags);
  auto circ = gal::Circle2d::minBoundingCircle(points);

  for (const auto& pt : points) {
    ASSERT_TRUE(circ.contains(pt));
  }
}
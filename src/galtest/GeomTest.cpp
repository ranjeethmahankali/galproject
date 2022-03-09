#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include <galcore/Annotations.h>
#include <galcore/Circle2d.h>
#include <galcore/Plane.h>
#include <galcore/Sphere.h>
#include <galcore/Util.h>
#include <galtest/TestUtils.h>

static constexpr float TOLERANCE = 0.0001f;

TEST(Circle2d, MinBoundingCircle)
{
  const static std::vector<std::vector<glm::vec2>> pointSets {{
                                                                {0.f, 0.f},
                                                                {1.1f, 0.f},
                                                                {0.25f, 0.25f},
                                                                {1.f, 1.f},
                                                                {-.3f, 1.f},
                                                              },
                                                              {
                                                                {1.f, 0.f},
                                                                {0.f, 0.f},
                                                                {3.f, 0.f},
                                                                {2.f, 0.f},
                                                                {4.f, 0.f},
                                                                {5.f, 0.f},
                                                              }};

  for (const auto& points : pointSets) {
    auto cloud = gal::PointCloud(points);
    auto circ  = gal::Circle2d::minBoundingCircle(points);

    for (const auto& pt : points) {
      ASSERT_TRUE(circ.contains(pt, TOLERANCE));
    }
  }

  static constexpr size_t nRandPts = 500;
  gal::Box2               box(glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f));
  std::vector<glm::vec2>  randPts(nRandPts);
  box.randomPoints(nRandPts, randPts.begin());

  auto circ = gal::Circle2d::minBoundingCircle(randPts);
  for (const auto& pt : randPts) {
    ASSERT_TRUE(circ.contains(pt, TOLERANCE));
  }
}

TEST(Sphere, MinBoundingSphere)
{
  const static std::vector<glm::vec3> points {
    {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}};

  auto sp = gal::Sphere::minBoundingSphere(points);
  for (const auto& pt : points) {
    ASSERT_TRUE(sp.contains(pt, TOLERANCE));
  }
}

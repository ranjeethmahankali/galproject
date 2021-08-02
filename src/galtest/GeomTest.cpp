#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <galcore/Annotations.h>
#include <galcore/Circle2d.h>
#include <galcore/DebugProfile.h>
#include <galcore/ObjLoader.h>
#include <galcore/Plane.h>
#include <galcore/Util.h>

static constexpr float TOLERANCE = 0.0001f;

TEST(Circle2d, MinBoundingCircle)
{
  GALSCOPE(__func__);
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
    GALCAPTURE(cloud);
    GALCAPTURE_WITH_NAME(gal::createIndexedPointCloud(cloud), tags);
    auto circ = gal::Circle2d::minBoundingCircle(points);

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
  GALSCOPE(__func__);
  const static std::vector<glm::vec3> points {
    {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}};

  auto cloud = gal::PointCloud(points);
  GALCAPTURE(cloud);
  GALCAPTURE_WITH_NAME(gal::createIndexedPointCloud(cloud), tags);
  auto sp = gal::Sphere::minBoundingSphere(points);

  for (const auto& pt : points) {
    ASSERT_TRUE(sp.contains(pt, TOLERANCE));
  }
}

TEST(Mesh, Area)
{
  auto mesh = gal::createRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  ASSERT_FLOAT_EQ(mesh.area(), 1.f);

  mesh = gal::io::ObjMeshData(gal::utils::absPath("../assets/bunny.obj"), true).toMesh();
  mesh.transform(glm::scale(glm::vec3(10.f)));
  ASSERT_FLOAT_EQ(mesh.area(), 5.646862f);
}

TEST(Mesh, Volume)
{
  auto mesh = gal::createRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  mesh =
    gal::io::ObjMeshData(gal::utils::absPath("../assets/bunny_large.obj"), true).toMesh();

  ASSERT_FLOAT_EQ(mesh.volume(), 6.0392118f);
}

TEST(Mesh, ClippedWithPlane)
{
  auto mesh =
    gal::io::ObjMeshData(gal::utils::absPath("../assets/bunny.obj"), true).toMesh();
  mesh.transform(glm::scale(glm::vec3(10.f)));

  auto clipped = mesh.clippedWithPlane(
    gal::Plane(glm::vec3 {.5f, .241f, .5f}, glm::vec3 {.5f, .638f, 1.f}));

  ASSERT_FLOAT_EQ(clipped.area(), 3.4405894f);
  ASSERT_FLOAT_EQ(clipped.volume(), 0.f);
}

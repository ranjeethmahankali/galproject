#include <gtest/gtest.h>
#include <glm/gtx/transform.hpp>

#include <galcore/Mesh.h>
#include <galcore/ObjLoader.h>
#include <galtest/TestUtils.h>

TEST(Mesh, Area)
{
  auto mesh = gal::makeRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  ASSERT_FLOAT_EQ(mesh.area(), 1.f);

  gal::fs::path fpath = GAL_ASSET_DIR / "bunny.obj";
  mesh                = gal::io::ObjMeshData(fpath, true).toTriMesh();
  mesh.transform(glm::scale(glm::vec3(10.f)));
  ASSERT_FLOAT_EQ(mesh.area(), 5.646862f);
}

TEST(Mesh, Volume)
{
  auto mesh = gal::io::ObjMeshData(GAL_ASSET_DIR / "bunny_large.obj", true).toTriMesh();
  ASSERT_FLOAT_EQ(mesh.volume(), 6.0392118f);
}

TEST(Mesh, ClippedWithPlane)
{
  auto mesh = gal::io::ObjMeshData(GAL_ASSET_DIR / "bunny.obj", true).toTriMesh();
  mesh.transform(glm::scale(glm::vec3(10.f)));

  auto clipped = mesh.clippedWithPlane(
    gal::Plane(glm::vec3 {.5f, .241f, .5f}, glm::vec3 {.5f, .638f, 1.f}));

  ASSERT_FLOAT_EQ(clipped.area(), 3.4405894f);
  ASSERT_FLOAT_EQ(clipped.volume(), 0.f);
}

TEST(Mesh, RectangleMesh)
{
  gal::Plane plane({0.f, 0.f, 0.f}, {1.f, 1.f, 0.f});
  auto rect = gal::makeRectangularMesh(plane, gal::Box2({0.f, 0.f}, {15.f, 12.f}), 1.f);
  ASSERT_EQ(180.f, rect.area());
  ASSERT_EQ(360, rect.n_faces());
  ASSERT_EQ(208, rect.n_vertices());
}

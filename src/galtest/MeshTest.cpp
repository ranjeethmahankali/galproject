#include <gtest/gtest.h>
#include <glm/gtx/transform.hpp>

#include <galcore/Mesh.h>
#include <galcore/ObjLoader.h>
#include <galtest/TestUtils.h>

gal::TriMesh unitbox()
{
  using namespace gal;
  TriMesh box;
  box.reserve(8, 18, 12);
  auto vs = std::array<TriMesh::VertH, 8> {
    box.add_vertex({0.f, 0.f, 0.f}),
    box.add_vertex({1.f, 0.f, 0.f}),
    box.add_vertex({1.f, 1.f, 0.f}),
    box.add_vertex({0.f, 1.f, 0.f}),
    box.add_vertex({0.f, 0.f, 1.f}),
    box.add_vertex({1.f, 0.f, 1.f}),
    box.add_vertex({1.f, 1.f, 1.f}),
    box.add_vertex({0.f, 1.f, 1.f}),
  };
  box.add_face(vs[0], vs[3], vs[2]);
  box.add_face(vs[0], vs[2], vs[1]);
  box.add_face(vs[0], vs[1], vs[4]);
  box.add_face(vs[4], vs[1], vs[5]);
  box.add_face(vs[3], vs[0], vs[4]);
  box.add_face(vs[3], vs[4], vs[7]);
  box.add_face(vs[4], vs[5], vs[6]);
  box.add_face(vs[4], vs[6], vs[7]);
  box.add_face(vs[3], vs[7], vs[2]);
  box.add_face(vs[2], vs[7], vs[6]);
  box.add_face(vs[2], vs[6], vs[5]);
  box.add_face(vs[2], vs[5], vs[1]);
  return box;
}

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
  auto mesh = gal::makeRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  ASSERT_FLOAT_EQ(mesh.volume(), 0.f);
  mesh = gal::io::ObjMeshData(GAL_ASSET_DIR / "bunny_large.obj", true).toTriMesh();
  ASSERT_FLOAT_EQ(mesh.volume(), 6.0392089f);
  ASSERT_FLOAT_EQ(unitbox().volume(), 1.f);
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

TEST(Mesh, Centroid)
{
  auto mesh = gal::io::ObjMeshData(GAL_ASSET_DIR / "bunny_large.obj", true).toTriMesh();
  ASSERT_NEAR(
    glm::distance({-0.533199f, -0.179856f, 0.898604f}, mesh.centroid()), 0.f, 1e-6);
}

TEST(Mesh, SphereQuery)
{
  auto mesh = gal::io::ObjMeshData(GAL_ASSET_DIR / "bunny.obj", true).toTriMesh();
  mesh.transform(glm::scale(glm::vec3(10.f)));
  gal::Sphere      sp(glm::vec3(0.f), 0.5f);
  std::vector<int> indices;
  mesh.querySphere(sp, std::back_inserter(indices), gal::eMeshElement::face);
  auto smesh = mesh.subMesh(indices);
  std::cout << smesh.n_faces() << " " << mesh.n_faces() << std::endl;
  ASSERT_FLOAT_EQ(smesh.area(), 0.44368880987167358);
}

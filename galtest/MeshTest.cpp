#include <catch2/catch_all.hpp>

#include <Mesh.h>
#include <TestUtils.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>
#include <chrono>
#include <glm/gtx/transform.hpp>
#include <numeric>

gal::TriMesh unitbox()
{
  using namespace gal;
  TriMesh box;
  box.reserve(8, 18, 12);
  std::array<VertH, 8>     vs;
  std::array<glm::vec3, 8> coords {{{0.f, 0.f, 0.f},
                                    {1.f, 0.f, 0.f},
                                    {1.f, 1.f, 0.f},
                                    {0.f, 1.f, 0.f},
                                    {0.f, 0.f, 1.f},
                                    {1.f, 0.f, 1.f},
                                    {1.f, 1.f, 1.f},
                                    {0.f, 1.f, 1.f}}};
  for (size_t i = 0; i < 8; ++i) {
    vs[i] = handle<VertH>(box.add_vertex(coords[i]));
  }
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

TEST_CASE("Mesh - Area", "[mesh][area]")  // NOLINT
{
  auto mesh = gal::makeRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  REQUIRE(Catch::Approx(mesh.area()) == 1.f);
  gal::fs::path fpath = GAL_ASSET_DIR / "bunny.obj";
  mesh                = gal::TriMesh::loadFromFile(fpath, true);
  mesh.transform(glm::scale(glm::vec3(10.f)));
  REQUIRE(Catch::Approx(mesh.area()) == 5.646862f);
}

TEST_CASE("Mesh - Subdivision", "[mesh][subdivision]")  // NOLINT
{
  gal::PolyMesh mesh;
  OpenMesh::IO::read_mesh(mesh, (GAL_ASSET_DIR / "bunny.obj").string());
  mesh.transform(glm::scale(glm::vec3(10.f)));
  OpenMesh::Subdivider::Uniform::CatmullClarkT<gal::PolyMesh, float> sub;
  sub.attach(mesh);
  auto before = std::chrono::high_resolution_clock::now();
  sub(3);
  auto duration = std::chrono::high_resolution_clock::now() - before;
  sub.detach();
  std::cout << "Subdivision took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
            << "ms\n";
  std::cout << "Subdivided mesh has " << mesh.n_faces() << " faces, " << mesh.n_edges()
            << " edges, and " << mesh.n_vertices() << " vertices.\n";
  mesh.triangulate();
  float area = std::accumulate(
    mesh.faces_begin(), mesh.faces_end(), 0.f, [&](float total, gal::FaceH f) {
      return total + mesh.calc_sector_area(mesh.halfedge_handle(f));
    });
  REQUIRE(Catch::Approx(5.5665889) == area);
}

TEST_CASE("Mesh - Volume", "[mesh][volume]")  // NOLINT
{
  auto mesh = gal::makeRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  REQUIRE(Catch::Approx(mesh.volume()) == 0.f);
  mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny_large.obj", true);
  REQUIRE(Catch::Approx(mesh.volume()) == 6.0392089f);
  REQUIRE(Catch::Approx(unitbox().volume()) == 1.f);
}

TEST_CASE("Mesh - ClippedWithPlane", "[mesh][clipping]")  // NOLINT
{
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny.obj", true);
  mesh.transform(glm::scale(glm::vec3(10.f)));
  auto clipped = mesh.clippedWithPlane(
    gal::Plane(glm::vec3 {.5f, .241f, .5f}, glm::vec3 {.5f, .638f, 1.f}));
  REQUIRE(Catch::Approx(clipped.area()) == 3.4405894f);
  REQUIRE(Catch::Approx(clipped.volume()) == 0.f);
}

TEST_CASE("Mesh - RectangleMesh", "[mesh][rectangle]")  // NOLINT
{
  gal::Plane plane({0.f, 0.f, 0.f}, {1.f, 1.f, 0.f});
  auto rect = gal::makeRectangularMesh(plane, gal::Box2({0.f, 0.f}, {15.f, 12.f}), 1.f);
  REQUIRE(180.f == rect.area());
  REQUIRE(360 == rect.n_faces());
  REQUIRE(208 == rect.n_vertices());
}

TEST_CASE("Mesh - Centroid", "[mesh][centroid]")  // NOLINT
{
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny_large.obj", true);
  REQUIRE(
    Catch::Approx(glm::distance({-0.533199f, -0.179856f, 0.898604f}, mesh.centroid()))
      .margin(1e-6) == 0.f);
}

TEST_CASE("Mesh - SphereQuery", "[mesh][query]")  // NOLINT
{
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny.obj", true);
  mesh.transform(glm::scale(glm::vec3(10.f)));
  gal::Sphere      sp(glm::vec3(0.f), 0.5f);
  std::vector<int> indices;
  mesh.querySphere(sp, std::back_inserter(indices), gal::eMeshElement::face);
  auto smesh = mesh.subMesh(indices);
  REQUIRE(Catch::Approx(smesh.area()) == 0.44368880987167358);
}

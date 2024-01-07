#include <CGAL/Dynamic_property_map.h>
#include <gtest/gtest.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <stdexcept>

#include <Mesh.h>
#include <TestUtils.h>

#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>

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
  mesh                = gal::TriMesh::loadFromFile(fpath, true);
  mesh.transform(glm::scale(glm::vec3(10.f)));
  ASSERT_FLOAT_EQ(mesh.area(), 5.646862f);
}

TEST(Mesh, Volume)
{
  auto mesh = gal::makeRectangularMesh(
    gal::Plane({0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}), gal::Box2({0.f, 0.f}, {1.f, 1.f}), 1.f);
  ASSERT_FLOAT_EQ(mesh.volume(), 0.f);
  mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny_large.obj", true);
  ASSERT_FLOAT_EQ(mesh.volume(), 6.0392089f);
  ASSERT_FLOAT_EQ(unitbox().volume(), 1.f);
}

TEST(Mesh, ClippedWithPlane)
{
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny.obj", true);
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
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny_large.obj", true);
  ASSERT_NEAR(
    glm::distance({-0.533199f, -0.179856f, 0.898604f}, mesh.centroid()), 0.f, 1e-6);
}

TEST(Mesh, SphereQuery)
{
  auto mesh = gal::TriMesh::loadFromFile(GAL_ASSET_DIR / "bunny.obj", true);
  mesh.transform(glm::scale(glm::vec3(10.f)));
  gal::Sphere      sp(glm::vec3(0.f), 0.5f);
  std::vector<int> indices;
  mesh.querySphere(sp, std::back_inserter(indices), gal::eMeshElement::face);
  auto smesh = mesh.subMesh(indices);
  ASSERT_FLOAT_EQ(smesh.area(), 0.44368880987167358);
}

TEST(Mesh, Temporary)
{
  using namespace CGAL;
  using Kernel  = Simple_cartesian<double>;
  using Mesh    = Surface_mesh<Kernel::Point_3>;
  using VId     = Mesh::Vertex_index;
  using FId     = Mesh::Face_index;
  using HId     = Mesh::halfedge_index;
  namespace smp = Surface_mesh_parameterization;
  Mesh mesh;
  if (!IO::read_polygon_mesh("/home/rnjth94/buffer/parametrization/manifold1.obj",
                             mesh)) {
    throw std::runtime_error("Cannot read mesh file");
  };
  HId bhd     = Polygon_mesh_processing::longest_border(mesh).first;
  using UVMap = Mesh::Property_map<VId, Kernel::Point_2>;
  UVMap uvmap = mesh.add_property_map<VId, Kernel::Point_2>("h:uv").first;
  smp::parameterize(mesh, bhd, uvmap);
  gal::TriMesh uvmesh;
  {
    uvmesh.reserve(mesh.num_vertices(), mesh.num_edges(), mesh.num_faces());
    for (VId v : mesh.vertices()) {
      Kernel::Point_2 uv = get(uvmap, v);
      uvmesh.add_vertex({uv.x(), uv.y(), 0.});
    }
    for (FId f : mesh.faces()) {
      std::array<gal::TriMesh::VertH, 3> fvs;
      auto src = mesh.vertices_around_face(mesh.halfedge(f));
      std::transform(src.begin(), src.end(), fvs.begin(), [&](VId v) {
        return gal::TriMesh::VertH(v.idx());
      });
      uvmesh.add_face(fvs[0], fvs[1], fvs[2]);
    }
  }
  OpenMesh::IO::write_mesh(uvmesh, "/home/rnjth94/buffer/parametrization/uvmesh1.obj");
}

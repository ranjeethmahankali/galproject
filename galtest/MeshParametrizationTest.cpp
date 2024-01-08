#include <gtest/gtest.h>

#include <CGAL/Dynamic_property_map.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_parameterization/Barycentric_mapping_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Square_border_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <algorithm>
#include <stdexcept>

#include <Mesh.h>
#include <TestUtils.h>

using namespace CGAL;
using Kernel      = Simple_cartesian<double>;
using SurfaceMesh = Surface_mesh<Kernel::Point_3>;
using VId         = SurfaceMesh::Vertex_index;
using FId         = SurfaceMesh::Face_index;
using HId         = SurfaceMesh::halfedge_index;
using UVMap       = SurfaceMesh::Property_map<VId, Kernel::Point_2>;
namespace SMP     = Surface_mesh_parameterization;

void exportUVMesh(const UVMap& uvmap, const SurfaceMesh& mesh)
{
  gal::TriMesh uvmesh;
  {
    uvmesh.reserve(mesh.num_vertices(), mesh.num_edges(), mesh.num_faces());
    for (VId v : mesh.vertices()) {
      Kernel::Point_2 uv = get(uvmap, v);
      uvmesh.add_vertex(glm::vec3(uv.x(), uv.y(), 0.));
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

TEST(Mesh, Parametrization)
{
  SurfaceMesh mesh;
  if (!IO::read_polygon_mesh("/home/rnjth94/buffer/parametrization/manifold1.obj",
                             mesh)) {
    throw std::runtime_error("Cannot read mesh file");
  };
  HId   bhd   = Polygon_mesh_processing::longest_border(mesh).first;
  UVMap uvmap = mesh.add_property_map<VId, Kernel::Point_2>("h:uv").first;
  typedef SMP::Square_border_uniform_parameterizer_3<SurfaceMesh> Border_parameterizer;
  typedef SMP::Barycentric_mapping_parameterizer_3<SurfaceMesh, Border_parameterizer>
                  Parameterizer;
  SMP::Error_code err = SMP::parameterize(mesh, Parameterizer(), bhd, uvmap);
  if (err != SMP::OK) {
    std::cerr << "Error: " << SMP::get_error_message(err) << std::endl;
    throw std::runtime_error(SMP::get_error_message(err));
  }
  exportUVMesh(uvmap, mesh);
}

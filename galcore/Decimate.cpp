#include <CGAL/Surface_mesh/Surface_mesh.h>
#include <Decimate.h>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

namespace gal {

TriMesh decimate(TriMesh mesh, int nCollapses)
{
  OpenMesh::Decimater::DecimaterT<TriMesh>          decimater(mesh);
  OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle quadric;
  decimater.add(quadric);
  decimater.module(quadric).unset_max_err();
  decimater.initialize();
  decimater.decimate(nCollapses);
  decimater.mesh().garbage_collection();
  return decimater.mesh();
}

using namespace CGAL;
using Kernel      = Simple_cartesian<double>;
using SurfaceMesh = Surface_mesh<Kernel::Point_3>;
using VId         = SurfaceMesh::Vertex_index;
using FId         = SurfaceMesh::Face_index;
using HId         = SurfaceMesh::halfedge_index;
using EId         = SurfaceMesh::Edge_index;

SurfaceMesh toCgal(const TriMesh& mesh)
{
  SurfaceMesh out;
  out.reserve(mesh.n_vertices(), mesh.n_edges(), mesh.n_faces());
  for (TriMesh::VertH v : mesh.vertices()) {
    const auto& p = mesh.point(v);
    out.add_vertex(Kernel::Point_3 {double(p[0]), double(p[1]), double(p[2])});
  }
  for (TriMesh::FaceH f : mesh.faces()) {
    std::array<VId, 3> vids;
    std::transform(
      mesh.cfv_begin(f), mesh.cfv_end(f), vids.begin(), [](TriMesh::VertH v) {
        return VId(v.idx());
      });
    out.add_face(vids[0], vids[1], vids[2]);
  }
  return out;
}

void fromCgal(const SurfaceMesh& src, TriMesh& dst)
{
  dst.clear();
  dst.reserve(src.num_vertices(), src.num_edges(), src.num_faces());
  for (VId v : src.vertices()) {
    const auto& p = src.point(v);
    dst.add_vertex(glm::vec3(p[0], p[1], p[2]));
  }
  for (FId f : src.faces()) {
    std::array<TriMesh::VertH, 3> vs;
    const auto&                   fvs = src.vertices_around_face(src.halfedge(f));
    std::transform(
      fvs.begin(), fvs.end(), vs.begin(), [](VId v) { return TriMesh::VertH(v.idx()); });
    dst.add_face(vs.data(), vs.size());
  }
  dst.setVertexColor({1.f, 1.f, 1.f});
}

TriMesh simplify(TriMesh omesh, int nCollapses)
{
  namespace SMS                                     = CGAL::Surface_mesh_simplification;
  auto                                   mesh       = toCgal(omesh);
  std::chrono::steady_clock::time_point  start_time = std::chrono::steady_clock::now();
  SMS::Count_stop_predicate<SurfaceMesh> stop(
    std::max(6, int(omesh.n_edges()) - nCollapses));
  SMS::edge_collapse(mesh, stop);
  mesh.collect_garbage();

  fromCgal(mesh, omesh);
  return omesh;
}

}  // namespace gal

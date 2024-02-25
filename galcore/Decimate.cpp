#include <Decimate.h>

#include <stdint.h>
#include <cmath>
#include <queue>
#include <span>

#include <tbb/parallel_for_each.h>
#include <OpenMesh/Core/Utils/Property.hh>

#include <Util.h>

namespace gal {

template<typename F>
void colorMesh(TriMesh& mesh, std::span<const glm::vec3> palette, F vfn)
{
  OpenMesh::VPropHandleT<float> vals;
  mesh.add_property(vals);
  float maxval = -FLT_MAX;
  float minval = FLT_MAX;
  // Compute the values at all vertices.
  for (TriMesh::VertH vh : mesh.vertices()) {
    float val               = vfn(vh);
    maxval                  = std::max(val, maxval);
    minval                  = std::min(val, minval);
    mesh.property(vals, vh) = val;
  }
  // Compute the color according to palette.
  for (TriMesh::VertH vh : mesh.vertices()) {
    float  fci = 0.f;
    float  t   = size_t(std::modf(
      float(palette.size() - 1) * (mesh.property(vals, vh) - minval) / (maxval - minval),
      &fci));
    size_t ci  = size_t(fci);
    mesh.set_color(vh,
                   (t == 0.f || ci == palette.size() - 1)
                     ? palette[ci]
                     : palette[ci] * (1.f - t) + palette[ci + 1] * t);
  }
  mesh.remove_property(vals);
}

static float edgeCost(const TriMesh& mesh, TriMesh::EdgeH eh)
{
  std::array<glm::vec3, 2> normals;
  for (uint32_t i = 0; i < 2; ++i) {
    // Assuming openmesh already normalized the normals. If that assumption
    // ever breaks in the future, consider normalizing them here.
    normals[i] = mesh.normal(mesh.to_vertex_handle(mesh.halfedge_handle(eh, i)));
  }
  return 1.f - glm::dot(normals[0], normals[1]);
}

static float vertexCost(const TriMesh& mesh, TriMesh::VertH vh)
{
  glm::vec3 vnorm  = mesh.normal(vh);
  float     maxdev = -FLT_MAX;
  for (TriMesh::FaceH fh : mesh.vf_range(vh)) {
    maxdev = std::max(maxdev, 1.f - glm::dot(vnorm, mesh.normal(fh)));
  }
  return maxdev;
}

TriMesh decimate(TriMesh mesh, int nCollapses)
{
  OpenMesh::EPropHandleT<float> ecost;
  OpenMesh::VPropHandleT<float> vcost;
  mesh.add_property(ecost);
  mesh.add_property(vcost);
  mesh.update_normals();
  // Compute edge costs.
  tbb::parallel_for_each(mesh.edges(), [&](TriMesh::EdgeH eh) {
    mesh.property(ecost, eh) = edgeCost(mesh, eh);
  });
  // Compute vertex normal deviations.
  tbb::parallel_for_each(mesh.vertices(), [&](TriMesh::VertH vh) {
    mesh.property(vcost, vh) = vertexCost(mesh, vh);
  });
  std::priority_queue<std::pair<TriMesh::EdgeH, float>> queue;
  for (TriMesh::EdgeH eh : mesh.edges()) {
    queue.push(std::make_pair(eh, mesh.property(ecost, eh)));
  }
  size_t maxCollapses = std::min(size_t(nCollapses), mesh.n_edges());
  int    collapsed    = 0;
  while (collapsed < maxCollapses && !queue.empty()) {
    auto [eh, cost] = queue.top();
    queue.pop();
    if (mesh.status(eh).deleted()) {
      continue;
    }
    // Compute the new position of the collapsed vertex.
    glm::vec3 newpos = {0.f, 0.f, 0.f};
    {
      float wsum = 0.f;
      for (uint32_t vi = 0; vi < 2; ++vi) {
        TriMesh::VertH vh  = mesh.to_vertex_handle(mesh.halfedge_handle(eh, vi));
        auto           dev = mesh.property(vcost, vh);
        auto           pt  = mesh.point(vh);
        newpos += dev * pt;
        wsum += dev;
      }
      newpos /= wsum;
    }
    // Do the collapse.
    TriMesh::HalfH he = mesh.halfedge_handle(eh, 0);
    TriMesh::VertH vh = mesh.to_vertex_handle(he);
    mesh.collapse(he);
    mesh.point(vh) = newpos;
    // Update normals in the neighborhood..
    for (TriMesh::FaceH fh : mesh.vf_range(vh)) {
      mesh.update_normal(fh);
    }
    mesh.update_normal(vh);
    for (TriMesh::VertH nvh : mesh.vv_range(vh)) {
      mesh.update_normal(nvh);
    }
    // Update the costs of the neighborhood
    for (TriMesh::EdgeH neh : mesh.ve_range(vh)) {
      mesh.property(ecost, neh) = edgeCost(mesh, neh);
    }
    for (TriMesh::VertH nvh : mesh.vv_range(vh)) {
      mesh.property(vcost, nvh) = vertexCost(mesh, nvh);
    }
    ++collapsed;
  }
  // Cleanup and return.
  mesh.remove_property(ecost);
  mesh.remove_property(vcost);
  mesh.garbage_collection();
  return mesh;
}

}  // namespace gal

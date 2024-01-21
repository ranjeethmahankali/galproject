#include <Decimate.h>
#include <tbb/parallel_for_each.h>

#include <stdint.h>
#include <OpenMesh/Core/Mesh/SmartHandles.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <cmath>
#include <glm/geometric.hpp>
#include <queue>
#include <span>
#include "Util.h"

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

TriMesh decimate(TriMesh mesh, int nCollapses)
{
  OpenMesh::EPropHandleT<float> enormdev;
  OpenMesh::VPropHandleT<float> vnormdev;
  mesh.add_property(enormdev);
  mesh.add_property(vnormdev);
  mesh.update_normals();
  // Compute edge normal deviations.
  tbb::parallel_for_each(mesh.edges(), [&](TriMesh::EdgeH eh) {
    std::array<glm::vec3, 2> normals;
    for (uint32_t i = 0; i < 2; ++i) {
      // Assuming openmesh already normalized the normals. If that assumption
      // ever breaks in the future, consider normalizing them here.
      normals[i] = mesh.normal(mesh.to_vertex_handle(mesh.halfedge_handle(eh, i)));
    }
    mesh.property(enormdev, eh) = 1.f - glm::dot(normals[0], normals[1]);
  });
  // Compute vertex normal deviations.
  tbb::parallel_for_each(mesh.vertices(), [&](TriMesh::VertH vh) {
    glm::vec3 vnorm  = mesh.normal(vh);
    float     maxdev = -FLT_MAX;
    for (TriMesh::FaceH fh : mesh.vf_range(vh)) {
      maxdev = std::max(maxdev, 1.f - glm::dot(vnorm, mesh.normal(fh)));
    }
    mesh.property(vnormdev, vh) = maxdev;
  });
  std::priority_queue<TriMesh::EdgeH> queue;
  // Cleanup and return.
  mesh.remove_property(enormdev);
  mesh.remove_property(vnormdev);
  return mesh;
}

}  // namespace gal

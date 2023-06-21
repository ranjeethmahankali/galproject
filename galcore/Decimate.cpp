#include <Decimate.h>

namespace gal {

TriMesh decimate(TriMesh mesh, int nCollapses)
{
  OpenMesh::Decimater::DecimaterT<TriMesh>          decimater(mesh);
  OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle quadric;
  // OpenMesh::Decimater::ModProgMeshT<TriMesh>::Handle prog;
  decimater.add(quadric);
  // decimater.add(prog);
  // decimater.module(quadric).set_max_err(0.1);
  decimater.module(quadric).unset_max_err();
  decimater.initialize();
  decimater.decimate(nCollapses);
  decimater.mesh().garbage_collection();
  return decimater.mesh();

  // TODO: Not implemented.
}

}  // namespace gal

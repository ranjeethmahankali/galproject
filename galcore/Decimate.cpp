#include <Decimate.h>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

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

}  // namespace gal

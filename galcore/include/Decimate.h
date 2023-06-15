#pragma once

#include <Mesh.h>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

namespace gal {

template<typename MeshOutIter>
void decimateWithHistory(TriMesh mesh, MeshOutIter outmeshes)
{
  *(outmeshes++) = mesh;
  OpenMesh::Decimater::DecimaterT<TriMesh>          decimater(mesh);
  OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle quadric;
  // OpenMesh::Decimater::ModProgMeshT<TriMesh>::Handle prog;
  decimater.add(quadric);
  // decimater.add(prog);
  decimater.module(quadric).set_max_err(0.0001);
  decimater.initialize();
  decimater.decimate();
  *(outmeshes++) = decimater.mesh();
  // TODO: Not implemented.
}

}  // namespace gal

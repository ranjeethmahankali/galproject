#pragma once

#include <Mesh.h>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

namespace gal {

TriMesh decimate(TriMesh mesh, int nCollapses);

}  // namespace gal

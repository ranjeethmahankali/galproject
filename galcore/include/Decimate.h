#pragma once

#include <Mesh.h>

namespace gal {

TriMesh decimate(TriMesh mesh, int nCollapses);

TriMesh simplify(TriMesh mesh, int nCollapses);

}  // namespace gal

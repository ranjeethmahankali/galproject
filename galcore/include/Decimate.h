#pragma once

#include <Mesh.h>

namespace gal {

template<typename MeshOutIter>
void decimateWithHistory(TriMesh mesh, MeshOutIter outmeshes)
{
  *(outmeshes++) = mesh;
  *(outmeshes++) = mesh;
  // TODO: Not implemented.
}

}  // namespace gal

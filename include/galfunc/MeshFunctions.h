#pragma once

#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>

TYPE_INFO(gal::Mesh, 0x45342367);

namespace gal {
namespace func {

GAL_FUNC_DECL(((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")),
              meshCentroid,
              true,
              3,
              "Gets the centroid of a mesh",
              (gal::Mesh, mesh, "The mesh"));

GAL_FUNC_DECL(((gal::Mesh, mesh, "Loaded mesh")),
              loadObjFile,
              true,
              1,
              "Loads a mesh from an obj file",
              (std::string, filepath, "The path to the obj file"));

}  // namespace func
}  // namespace gal
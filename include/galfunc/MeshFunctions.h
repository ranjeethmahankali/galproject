#pragma once

#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>
#include <galfunc/GeomFunctions.h>

GAL_TYPE_INFO(gal::Mesh, 0x45342367);

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

GAL_FUNC_DECL(((gal::Mesh, mesh, "Input mesh")),
              scaleMesh,
              true,
              2,
              "Scales the mesh. Returns a new instance.",
              (gal::Mesh, mesh, "Scaled mesh"),
              (float, scale, "Scale"));

GAL_FUNC_DECL(((gal::Mesh, mesh, "Clipped mesh")),
              clipMesh,
              true,
              2,
              "Clips the given mesh with the plane. Returns a new mesh.",
              (gal::Mesh, mesh, "mesh to clip"),
              (gal::Plane, plane, "Plane to clip with"));

}  // namespace func
}  // namespace gal
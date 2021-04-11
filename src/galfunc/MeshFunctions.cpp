#include <galfunc/MeshFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")),
              meshCentroid,
              true,
              1,
              "Gets the centroid of a mesh",
              (gal::Mesh, mesh, "The mesh"))
{
  auto pt = mesh->centroid(gal::eMeshCentroidType::volumeBased);
  return std::make_tuple(std::make_shared<float>(pt.x),
                         std::make_shared<float>(pt.y),
                         std::make_shared<float>(pt.z));
};

GAL_FUNC_DEFN(((gal::Mesh, mesh, "Loaded mesh")),
              loadObjFile,
              true,
              1,
              "Loads a mesh from an obj file",
              (std::string, filepath, "The path to the obj file"))
{
  return std::make_tuple(
    std::make_shared<gal::Mesh>(io::ObjMeshData(*filepath, true).toMesh()));
};

}  // namespace func
}  // namespace gal
#include <galfunc/MeshFunctions.h>
#include <glm/gtx/transform.hpp>

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

GAL_FUNC_DEFN(((gal::Mesh, mesh, "Input mesh")),
              scaleMesh,
              true,
              2,
              "Transforms the mesh. Modifies the given instance",
              (gal::Mesh, mesh, "Transformed mesh"),
              (float, scale, "Scale"))
{
  float s = *scale;
  auto mesh2 = std::make_shared<gal::Mesh>(*mesh);
  mesh2->transform(glm::scale(glm::vec3(s, s, s)));
  return std::make_tuple(mesh2);
};

GAL_FUNC_DEFN(((gal::Mesh, mesh, "Clipped mesh")),
              clipMesh,
              true,
              2,
              "Clips the given mesh with the plane. Returns a new mesh",
              (gal::Mesh, mesh, "mesh to clip"),
              (gal::Plane, plane, "Plane to clip with"))
{
  auto mesh2 = std::make_shared<gal::Mesh>(*mesh);
  mesh2->clipWithPlane(*plane);
  return std::make_tuple(mesh2);
};

}  // namespace func
}  // namespace gal
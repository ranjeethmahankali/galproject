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
  float s     = *scale;
  auto  mesh2 = std::make_shared<gal::Mesh>(*mesh);
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

GAL_FUNC_DEFN(
  ((gal::Mesh, resultMesh, "Mesh with the queried faces")),
  meshSphereQuery,
  true,
  2,
  "Queries the mesh face rtree with the given sphere and returns the new sub-mesh",
  (gal::Mesh, mesh, "Mesh to query"),
  (gal::Sphere, sphere, "Sphere to query the faces with"))
{
  std::vector<size_t> results;
  mesh->querySphere(*sphere, std::back_inserter(results), gal::eMeshElement::face);
  return std::make_tuple(std::make_shared<gal::Mesh>(mesh->extractFaces(results)));
};

GAL_FUNC_DEFN(((gal::PointCloud, outCloud, "Result point cloud")),
              closestPointsOnMesh,
              true,
              2,
              "Creates the result point cloud by closest-point-querying the mesh with "
              "the given point cloud",
              (gal::Mesh, mesh, "Mesh"),
              (gal::PointCloud, inCloud, "Query point cloud"))
{
  auto outCloud = std::make_shared<gal::PointCloud>();
  outCloud->reserve(inCloud->size());
  auto pbegin = inCloud->cbegin();
  auto pend   = inCloud->cend();
  while (pbegin != pend) {
    outCloud->push_back(mesh->closestPoint(*(pbegin++), FLT_MAX));
  }
  return std::make_tuple(outCloud);
};

GAL_FUNC_DEFN(((gal::Box3, bounds, "Bounds of the mesh")),
              meshBbox,
              true,
              1,
              "Gets the bounding box of the mesh",
              (gal::Mesh, mesh, "Mesh"))
{
  return std::make_tuple(std::make_shared<gal::Box3>(std::move(mesh->bounds())));
};

}  // namespace func
}  // namespace gal
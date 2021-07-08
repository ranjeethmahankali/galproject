#include <galfunc/MeshFunctions.h>
#include <glm/gtx/transform.hpp>

namespace gal {
namespace func {

GAL_FUNC_DEFN(((glm::vec3, centroid, "x coordinate")),
              meshCentroid,
              true,
              1,
              "Gets the centroid of a mesh",
              (gal::Mesh, mesh, "The mesh"))
{
  return std::make_tuple(
    std::make_shared<glm::vec3>(mesh->centroid(gal::eMeshCentroidType::volumeBased)));
};

GAL_FUNC_DEFN(((float, volume, "Volume of the mesh")),
              meshVolume,
              true,
              1,
              "Gets the volume of the mesh",
              (gal::Mesh, mesh, "The mesh"))
{
  return std::make_tuple(std::make_shared<float>(mesh->volume()));
};

GAL_FUNC_DEFN(((float, area, "Surface area of the mesh")),
              meshSurfaceArea,
              true,
              1,
              "Gets the surface area of the mesh",
              (gal::Mesh, mesh, "The mesh"))
{
  return std::make_tuple(std::make_shared<float>(mesh->area()));
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
  ((gal::Mesh, resultMesh, "Mesh with the queried faces"),
   (std::vector<int32_t>,
    faceIndices,
    "Indices of the faces that are inside / near the query sphere"),
   (int32_t, numFaces, "The number of faces in the query results")),
  meshSphereQuery,
  true,
  2,
  "Queries the mesh face rtree with the given sphere and returns the new sub-mesh",
  (gal::Mesh, mesh, "Mesh to query"),
  (gal::Sphere, sphere, "Sphere to query the faces with"))
{
  std::vector<size_t> results;
  mesh->querySphere(*sphere, std::back_inserter(results), gal::eMeshElement::face);
  std::vector<int32_t> indices(results.size());
  std::transform(
    results.begin(), results.end(), indices.begin(), [](size_t i) { return int32_t(i); });
  return std::make_tuple(std::make_shared<gal::Mesh>(mesh->extractFaces(results)),
                         std::make_shared<std::vector<int32_t>>(std::move(indices)),
                         std::make_shared<int32_t>(results.size()));
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
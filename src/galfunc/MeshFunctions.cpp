#include <galfunc/MeshFunctions.h>
#include <glm/gtx/transform.hpp>

namespace gal {
namespace func {

GAL_FUNC_DEFN(meshCentroid, ((gal::Mesh, mesh)), ((glm::vec3, centroid)))
{
  centroid = mesh.centroid(gal::eMeshCentroidType::volumeBased);
};

GAL_FUNC_DEFN(meshVolume, ((gal::Mesh, mesh)), ((float, volume)))
{
  volume = mesh.volume();
};

GAL_FUNC_DEFN(meshSurfaceArea, ((gal::Mesh, mesh)), ((float, area)))
{
  area = mesh.area();
};

GAL_FUNC_DEFN(loadObjFile, ((std::string, filepath)), ((gal::Mesh, mesh)))
{
  mesh = io::ObjMeshData(filepath, true).toMesh();
};

GAL_FUNC_DEFN(scaleMesh, ((gal::Mesh, mesh), (float, scale)), ((gal::Mesh, scaled)))
{
  scaled = mesh;
  scaled.transform(glm::scale(glm::vec3(scale)));
};

GAL_FUNC_DEFN(clipMesh, ((gal::Mesh, mesh), (gal::Plane, plane)), ((gal::Mesh, clipped)))
{
  clipped = mesh.clippedWithPlane(plane);
};

GAL_FUNC_DEFN(meshSphereQuery,
              ((gal::Mesh, mesh), (gal::Sphere, sphere)),
              ((gal::Mesh, resultMesh),
               (std::vector<int32_t>, faceIndices),
               (int32_t, numFaces)))
{
  std::vector<size_t> results;
  mesh.querySphere(sphere, std::back_inserter(results), gal::eMeshElement::face);
  faceIndices.resize(results.size());
  std::transform(results.begin(), results.end(), faceIndices.begin(), [](size_t i) {
    return int32_t(i);
  });
  resultMesh = mesh.extractFaces(results);
  numFaces   = int(results.size());
};

GAL_FUNC_DEFN(closestPointsOnMesh,
              ((gal::Mesh, mesh), (gal::PointCloud, inCloud)),
              ((gal::PointCloud, outCloud)))
{
  outCloud.clear();
  outCloud.reserve(inCloud.size());
  auto pbegin = inCloud.cbegin();
  auto pend   = inCloud.cend();
  while (pbegin != pend) {
    outCloud.push_back(mesh.closestPoint(*(pbegin++), FLT_MAX));
  }
};

GAL_FUNC_DEFN(meshBbox, ((gal::Mesh, mesh)), ((gal::Box3, bounds)))
{
  bounds = mesh.bounds();
};

GAL_FUNC_DEFN(rectangleMesh,
              ((gal::Plane, plane), (gal::Box2, bounds), (float, edgeLength)),
              ((gal::Mesh, mesh)))
{
  mesh = std::move(createRectangularMesh(plane, bounds, edgeLength));
}

}  // namespace func
}  // namespace gal

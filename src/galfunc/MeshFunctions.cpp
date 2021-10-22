#include <galfunc/MeshFunctions.h>
#include <tbb/parallel_for.h>
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
               ((data::WriteView<int32_t, 1>), faceIndices),
               (int32_t, numFaces)))
{
  // TODO: Refactor this to not require this vector (avoid allocation).
  std::vector<size_t> results;
  mesh.querySphere(sphere, std::back_inserter(results), gal::eMeshElement::face);
  faceIndices.reserve(results.size());
  std::transform(
    results.begin(), results.end(), std::back_inserter(faceIndices), [](size_t i) {
      return int32_t(i);
    });
  resultMesh = mesh.extractFaces(results);
  numFaces   = int32_t(results.size());
};

GAL_FUNC_DEFN(closestPointsOnMesh,
              ((gal::Mesh, mesh), ((data::ReadView<glm::vec3, 1>), inCloud)),
              (((data::WriteView<glm::vec3, 1>), outCloud)))
{
  outCloud.resize(inCloud.size());
  tbb::parallel_for(size_t(0), inCloud.size(), [&](size_t i) {
    glm::vec3 pt = inCloud[i];
    outCloud[i]  = mesh.closestPoint(pt, FLT_MAX);
  });
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

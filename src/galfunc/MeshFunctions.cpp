#include <tbb/parallel_for.h>
#include <glm/gtx/transform.hpp>
#include "galcore/ObjLoader.h"

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(centroid,
         "Gets the centroid of a mesh",
         ((gal::Mesh, mesh, "The mesh")),
         ((glm::vec3, centroid, "x coordinate")))
{
  centroid = mesh.centroid(gal::eMeshCentroidType::volumeBased);
}

GAL_FUNC(volume,
         "Gets the volume of the mesh",
         ((gal::Mesh, mesh, "The mesh")),
         ((float, volume, "Volume of the mesh")))
{
  volume = mesh.volume();
}

GAL_FUNC(area,
         "Gets the surface area of the mesh",
         ((gal::Mesh, mesh, "The mesh")),
         ((float, result, "Surface area of the mesh")))
{
  result = mesh.area();
}

GAL_FUNC(loadObjFile,
         "Loads a mesh from an obj file",
         ((std::string, filepath, "The path to the obj file")),
         ((gal::Mesh, mesh, "Loaded mesh")))
{
  mesh = io::ObjMeshData(filepath, true).toMesh();
}

GAL_FUNC(scale,
         "Scales the mesh. Returns a new instance.",
         ((gal::Mesh, mesh, "Scaled mesh"), (float, scale, "Scale")),
         ((gal::Mesh, scaled, "Input mesh")))
{
  scaled = mesh;
  scaled.transform(glm::scale(glm::vec3(scale)));
}

GAL_FUNC(clipMesh,
         "Clips the given mesh with the plane. Returns a new mesh.",
         ((gal::Mesh, mesh, "mesh to clip"), (gal::Plane, plane, "Plane to clip with")),
         ((gal::Mesh, clipped, "Clipped mesh")))
{
  clipped = mesh.clippedWithPlane(plane);
}

GAL_FUNC(meshSphereQuery,
         "Queries the mesh face rtree with the given sphere and "
         "returns the new sub-mesh",
         ((gal::Mesh, mesh, "Mesh to query"),
          (gal::Sphere, sphere, "Sphere to query the faces with")),
         ((gal::Mesh, resultMesh, "Mesh with the queried faces"),
          ((data::WriteView<int32_t, 1>),
           faceIndices,
           "Indices of the faces that are inside / near the query sphere"),
          (int32_t, numFaces, "The number of faces in the query results")))
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
}

GAL_FUNC(closestPoints,
         "Creates the result point cloud by closest-point-querying the mesh with "
         "the given point cloud",
         ((gal::Mesh, mesh, "Mesh"),
          ((data::ReadView<glm::vec3, 1>), inCloud, "Query point cloud")),
         (((data::WriteView<glm::vec3, 1>), outCloud, "Result point cloud")))
{
  outCloud.resize(inCloud.size());
  tbb::parallel_for(size_t(0), inCloud.size(), [&](size_t i) {
    glm::vec3 pt = inCloud[i];
    outCloud[i]  = mesh.closestPoint(pt, FLT_MAX);
  });
}

GAL_FUNC(bounds,
         "Gets the bounding box of the mesh",
         ((gal::Mesh, mesh, "Mesh")),
         ((gal::Box3, bbox, "Bounds of the mesh")))
{
  bbox = mesh.bounds();
}

GAL_FUNC(numFaces,
         "Gets the number of faces of the mesh",
         ((gal::Mesh, mesh, "Mesh")),
         ((int32_t, nfaces, "Number of faces")))
{
  nfaces = mesh.numFaces();
}

GAL_FUNC(numVertices,
         "Gets the number of vertices of the mesh",
         ((gal::Mesh, mesh, "Mesh")),
         ((int32_t, nverts, "Number of vertices")))
{
  nverts = mesh.numVertices();
}

GAL_FUNC(rectangleMesh,
         "Creates a rectangular mesh",
         ((gal::Plane, plane, "plane"),
          (gal::Box2, bounds, "Bounds"),
          (float, edgeLength, "Approximate edge length.")),
         ((gal::Mesh, mesh, "Resulting rectangular mesh")))
{
  mesh = std::move(createRectangularMesh(plane, bounds, edgeLength));
}

void bind_MeshFunctions()
{
  GAL_FN_BIND(centroid,
              volume,
              area,
              scale,
              bounds,
              numFaces,
              numVertices,
              loadObjFile,
              clipMesh,
              meshSphereQuery,
              closestPoints,
              rectangleMesh);
}

}  // namespace func
}  // namespace gal

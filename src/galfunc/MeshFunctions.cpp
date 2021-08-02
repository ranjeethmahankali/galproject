#include <galfunc/MeshFunctions.h>
#include <glm/gtx/transform.hpp>

namespace gal {
namespace func {

GAL_FUNC_DEFN(meshCentroid,
              1,
              1,
              "Gets the centroid of a mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((glm::vec3, centroid, "x coordinate")))
{
  *centroid = mesh->centroid(gal::eMeshCentroidType::volumeBased);
};

GAL_FUNC_DEFN(meshVolume,
              1,
              1,
              "Gets the volume of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, volume, "Volume of the mesh")))
{
  *volume = mesh->volume();
};

GAL_FUNC_DEFN(meshSurfaceArea,
              1,
              1,
              "Gets the surface area of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, area, "Surface area of the mesh")))
{
  *area = mesh->area();
};

GAL_FUNC_DEFN(loadObjFile,
              1,
              1,
              "Loads a mesh from an obj file",
              ((std::string, filepath, "The path to the obj file")),
              ((gal::Mesh, mesh, "Loaded mesh")))
{
  *mesh = io::ObjMeshData(*filepath, true).toMesh();
};

GAL_FUNC_DEFN(scaleMesh,
              2,
              1,
              "Scales the mesh. Returns a new instance.",
              ((gal::Mesh, mesh, "Scaled mesh"), (float, scale, "Scale")),
              ((gal::Mesh, scaled, "Input mesh")))
{
  *scaled = *mesh;
  scaled->transform(glm::scale(glm::vec3(*scale)));
};

GAL_FUNC_DEFN(clipMesh,
              2,
              1,
              "Clips the given mesh with the plane. Returns a new mesh.",
              ((gal::Mesh, mesh, "mesh to clip"),
               (gal::Plane, plane, "Plane to clip with")),
              ((gal::Mesh, clipped, "Clipped mesh")))
{
  *clipped = mesh->clippedWithPlane(*plane);
};

GAL_FUNC_DEFN(meshSphereQuery,
              2,
              3,
              "Queries the mesh face rtree with the given sphere and "
              "returns the new sub-mesh",
              ((gal::Mesh, mesh, "Mesh to query"),
               (gal::Sphere, sphere, "Sphere to query the faces with")),
              ((gal::Mesh, resultMesh, "Mesh with the queried faces"),
               (std::vector<int32_t>,
                faceIndices,
                "Indices of the faces that are inside / near the query sphere"),
               (int32_t, numFaces, "The number of faces in the query results")))
{
  std::vector<size_t> results;
  mesh->querySphere(*sphere, std::back_inserter(results), gal::eMeshElement::face);
  faceIndices->resize(results.size());
  std::transform(results.begin(), results.end(), faceIndices->begin(), [](size_t i) {
    return int32_t(i);
  });
  *resultMesh = mesh->extractFaces(results);
  *numFaces   = int(results.size());
};

GAL_FUNC_DEFN(closestPointsOnMesh,
              2,
              1,
              "Creates the result point cloud by closest-point-querying the mesh with "
              "the given point cloud",
              ((gal::Mesh, mesh, "Mesh"),
               (gal::PointCloud, inCloud, "Query point cloud")),
              ((gal::PointCloud, outCloud, "Result point cloud")))
{
  outCloud->clear();
  outCloud->reserve(inCloud->size());
  auto pbegin = inCloud->cbegin();
  auto pend   = inCloud->cend();
  while (pbegin != pend) {
    outCloud->push_back(mesh->closestPoint(*(pbegin++), FLT_MAX));
  }
};

GAL_FUNC_DEFN(meshBbox,
              1,
              1,
              "Gets the bounding box of the mesh",
              ((gal::Mesh, mesh, "Mesh")),
              ((gal::Box3, bounds, "Bounds of the mesh")))
{
  *bounds = mesh->bounds();
};

GAL_FUNC_DEFN(rectangleMesh,
              3,
              1,
              "Creates a rectangular mesh",
              ((gal::Plane, plane, "plane"),
               (gal::Box2, bounds, "Bounds"),
               (float, edgeLength, "Approximate edge length.")),
              ((gal::Mesh, mesh, "Resulting rectangular mesh")))
{
  *mesh = std::move(createRectangularMesh(*plane, *bounds, *edgeLength));
}

GAL_FUNC_DEFN(meshWithVertexColorsFromLambda,
              2,
              1,
              "Assigns mesh vertex colors using the lambda.",
              ((gal::Mesh, inMesh, "Input mesh"),
               (gal::func::store::Lambda,
                colorFn,
                "Subgraph to be used as a lambda to compute vertex colors. Lambda should "
                "have one argument of type point and should output a 3-component vector "
                "representing the color")),
              ((gal::Mesh, outMesh, "Output mesh")))
{
  *outMesh                          = *inMesh;
  size_t                     nVerts = outMesh->numVertices();
  std::shared_ptr<glm::vec3> vert   = std::make_shared<glm::vec3>();
  for (size_t vi = 0; vi < nVerts; vi++) {
    *vert = outMesh->vertex(vi);
    colorFn->setInput(0, vert);
    auto color = colorFn->getOutput<glm::vec3>(0);
    outMesh->vertexColor(*color, vi);
  }
}

}  // namespace func
}  // namespace gal

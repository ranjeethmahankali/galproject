#include <tbb/parallel_for.h>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>

#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC(centroid,
         "Gets the centroid of a mesh",
         ((gal::TriMesh, mesh, "The mesh")),
         ((glm::vec3, centroid, "x coordinate")))
{
  centroid = mesh.centroid(gal::eMeshCentroidType::volumeBased);
}

GAL_FUNC(volume,
         "Gets the volume of the mesh",
         ((gal::TriMesh, mesh, "The mesh")),
         ((float, volume, "Volume of the mesh")))
{
  volume = mesh.volume();
}

GAL_FUNC(area,
         "Gets the surface area of the mesh",
         ((gal::TriMesh, mesh, "The mesh")),
         ((float, result, "Surface area of the mesh")))
{
  result = mesh.area();
}

GAL_FUNC(loadObjFile,
         "Loads a mesh from an obj file",
         ((std::string, filepath, "The path to the obj file")),
         ((gal::TriMesh, mesh, "Loaded mesh")))
{
  mesh = io::ObjMeshData(filepath, true).toTriMesh();
}

GAL_FUNC(scale,
         "Scales the mesh. Returns a new instance.",
         ((gal::TriMesh, mesh, "Scaled mesh"), (float, scale, "Scale")),
         ((gal::TriMesh, scaled, "Input mesh")))
{
  scaled = mesh;
  scaled.transform(glm::scale(glm::vec3(scale)));
}

GAL_FUNC(clipMesh,
         "Clips the given mesh with the plane. Returns a new mesh.",
         ((gal::TriMesh, mesh, "mesh to clip"),
          (gal::Plane, plane, "Plane to clip with")),
         ((gal::TriMesh, clipped, "Clipped mesh")))
{
  clipped = mesh.clippedWithPlane(plane);
}

GAL_FUNC(meshSphereQuery,
         "Queries the mesh face rtree with the given sphere and "
         "returns the new sub-mesh",
         ((gal::TriMesh, mesh, "Mesh to query"),
          (gal::Sphere, sphere, "Sphere to query the faces with")),
         ((gal::TriMesh, resultMesh, "Mesh with the queried faces"),
          ((data::WriteView<int32_t, 1>),
           faceIndices,
           "Indices of the faces that are inside / near the query sphere"),
          (int32_t, numFaces, "The number of faces in the query results")))
{
  // TODO: Refactor this to not require this vector (avoid allocation).
  std::vector<int> results;
  mesh.querySphere(sphere, std::back_inserter(results), gal::eMeshElement::face);
  faceIndices.reserve(results.size());
  std::copy(results.begin(), results.end(), std::back_inserter(faceIndices));
  resultMesh = mesh.subMesh(results);
  numFaces   = int32_t(results.size());
}

GAL_FUNC(closestPoints,
         "Creates the result point cloud by closest-point-querying the mesh with "
         "the given point cloud",
         ((gal::TriMesh, mesh, "Mesh"),
          ((data::ReadView<glm::vec3, 1>), inCloud, "Query point cloud")),
         (((data::WriteView<glm::vec3, 1>), outCloud, "Result point cloud")))
{
  mesh.updateRTrees();
  outCloud.resize(inCloud.size());
  tbb::parallel_for(size_t(0), inCloud.size(), [&](size_t i) {
    outCloud[i] = mesh.closestPoint(inCloud[i], FLT_MAX);
  });
}

GAL_FUNC(bounds,
         "Gets the bounding box of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         ((gal::Box3, bbox, "Bounds of the mesh")))
{
  bbox = mesh.bounds();
}

GAL_FUNC(numFaces,
         "Gets the number of faces of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         ((int32_t, nfaces, "Number of faces")))
{
  nfaces = mesh.n_faces();
}

GAL_FUNC(numVertices,
         "Gets the number of vertices of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         ((int32_t, nverts, "Number of vertices")))
{
  nverts = mesh.n_vertices();
}

GAL_FUNC(vertices,
         "Gets the vertices of the mesh as a list of points",
         ((gal::TriMesh, mesh, "Mesh")),
         (((data::WriteView<glm::vec3, 1>), points, "Vertex positions")))
{
  points.reserve(mesh.n_vertices());
  std::transform(mesh.vertices().begin(),
                 mesh.vertices().end(),
                 std::back_inserter(points),
                 [&](TriMesh::VertH v) { return mesh.point(v); });
}

GAL_FUNC(rectangleMesh,
         "Creates a rectangular mesh",
         ((gal::Plane, plane, "plane"),
          (gal::Box2, bounds, "Bounds"),
          (float, edgeLength, "Approximate edge length.")),
         ((gal::TriMesh, mesh, "Resulting rectangular mesh")))
{
  mesh = std::move(makeRectangularMesh(plane, bounds, edgeLength));
}

GAL_FUNC(meshWithVertexColors,
         "Creates a new mesh by with the given vertex colors",
         ((gal::TriMesh, mesh, "Input mesh"),
          ((data::ReadView<glm::vec3, 1>), colors, "Vertex colors")),
         ((gal::TriMesh, outmesh, "Colored mesh with vertex colors.")))
{
  outmesh = mesh;
  outmesh.request_vertex_colors();
  for (int i = 0; i < colors.size(); i++) {
    outmesh.set_color(outmesh.vertex_handle(i), colors[i]);
  }
}

void bind_MeshFunc(py::module& module)
{
  GAL_FN_BIND(centroid, module);
  GAL_FN_BIND(volume, module);
  GAL_FN_BIND(area, module);
  GAL_FN_BIND(scale, module);
  GAL_FN_BIND(bounds, module);
  GAL_FN_BIND(numFaces, module);
  GAL_FN_BIND(numVertices, module);
  GAL_FN_BIND(vertices, module);
  GAL_FN_BIND(loadObjFile, module);
  GAL_FN_BIND(clipMesh, module);
  GAL_FN_BIND(meshSphereQuery, module);
  GAL_FN_BIND(closestPoints, module);
  GAL_FN_BIND(rectangleMesh, module);
  GAL_FN_BIND(meshWithVertexColors, module);
}

}  // namespace func
}  // namespace gal

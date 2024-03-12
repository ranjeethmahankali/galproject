#include <tbb/parallel_for.h>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>

#include <Data.h>
#include <Decimate.h>
#include <Functions.h>
#include <Line.h>
#include <Mesh.h>

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

GAL_FUNC(loadTriangleMesh,
         "Loads a triangle mesh from an obj file",
         ((std::string, filepath, "The path to the obj file")),
         ((gal::TriMesh, mesh, "Loaded mesh")))
{
  mesh = TriMesh::loadFromFile(filepath);
}

GAL_FUNC(loadPolyMesh,
         "Loads a polygon mesh from an obj file",
         ((std::string, filepath, "The path to the obj file")),
         ((gal::PolyMesh, mesh, "Loaded mesh")))
{
  mesh = PolyMesh::loadFromFile(filepath);
}

GAL_FUNC_TEMPLATE(((typename, MeshT)),
                  scale,
                  "Scales the mesh. Returns a new instance.",
                  ((MeshT, mesh, "Scaled mesh"), (float, scale, "Scale")),
                  ((MeshT, scaled, "Input mesh")))
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

GAL_FUNC(subTriMesh,
         "Gets a mesh with the subset of the faces of the input mesh",
         ((gal::TriMesh, mesh, "Input mesh"),
          ((data::ReadView<int32_t, 1>), indices, "Faces to copy into the output mesh")),
         ((gal::TriMesh, resultMesh, "Resulting mesh with the subset of faces")))
{
  resultMesh = mesh.subMesh(std::span<const int32_t>(indices.data(), indices.size()));
}

GAL_FUNC(subPolyMesh,
         "Get a mesh with the subset of faces of the input mesh",
         ((gal::PolyMesh, mesh, "Polygon mesh"),
          ((data::ReadView<int32_t, 1>), indices, "Faces to copy into the output mesh")),
         ((gal::PolyMesh, resultMesh, "Resulting mesh with the subset of faces")))
{
  resultMesh = mesh.subMesh(std::span<const int32_t>(indices.data(), indices.size()));
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

GAL_FUNC(boundsTriMesh,
         "Gets the bounding box of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         ((gal::Box3, bbox, "Bounds of the mesh")))
{
  bbox = mesh.bounds();
}

GAL_FUNC(boundsPolyMesh,
         "Gets the bounding box of the mesh",
         ((gal::PolyMesh, mesh, "Mesh")),
         ((gal::Box3, bbox, "Bounds of the mesh")))
{
  bbox = mesh.bounds();
}

GAL_FUNC(numTriMeshFaces,
         "Gets the number of faces of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         ((int32_t, nfaces, "Number of faces")))
{
  nfaces = mesh.n_faces();
}

GAL_FUNC(numPolyMeshFaces,
         "Gets the number of faces of the mesh",
         ((gal::PolyMesh, mesh, "Mesh")),
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

GAL_FUNC(vertexTriMesh,
         "Get the position of the mesh vertex",
         ((gal::TriMesh, mesh, "Mesh"), (int32_t, index, "The index of the vertex")),
         ((glm::vec3, point, "Vertex position")))
{
  point = mesh.point(TriMesh::VertH(index));
}

GAL_FUNC(vertexPolyMesh,
         "Get the position of the mesh vertex",
         ((gal::PolyMesh, mesh, "Mesh"), (int32_t, index, "The index of the vertex")),
         ((glm::vec3, point, "Vertex position")))
{
  point = mesh.point(TriMesh::VertH(index));
}

GAL_FUNC(halfedgeTriMesh,
         "Get the halfedge of the mesh as a line segment",
         ((gal::TriMesh, mesh, "Mesh"), (int32_t, index, "Index of the halfedge")),
         ((gal::Line3d, edge, "Line segment")))
{
  auto he = TriMesh::HalfH(index);
  edge    = gal::Line3d {mesh.point(mesh.from_vertex_handle(he)),
                      mesh.point(mesh.to_vertex_handle(he))};
}

GAL_FUNC(halfedgePolyMesh,
         "Get the halfedge of the mesh as a line segment",
         ((gal::PolyMesh, mesh, "Mesh"), (int32_t, index, "Index of the halfedge")),
         ((gal::Line3d, edge, "Line segment")))
{
  auto he = TriMesh::HalfH(index);
  edge    = gal::Line3d {mesh.point(mesh.from_vertex_handle(he)),
                      mesh.point(mesh.to_vertex_handle(he))};
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

GAL_FUNC(triMeshWithVertexColors,
         "Creates a new mesh by with the given vertex colors",
         ((gal::TriMesh, mesh, "Input mesh"),
          ((data::ReadView<glm::vec3, 1>), colors, "Vertex colors")),
         ((gal::TriMesh, outmesh, "Colored mesh with vertex colors.")))
{
  outmesh = mesh;
  outmesh.request_vertex_colors();
  for (TriMesh::VertH vh : outmesh.vertices()) {
    outmesh.set_color(vh, colors[std::min(vh.idx(), int(colors.size() - 1))]);
  }
}

GAL_FUNC(polyMeshWithVertexColors,
         "Creates a new mesh by with the given vertex colors",
         ((gal::PolyMesh, mesh, "Input mesh"),
          ((data::ReadView<glm::vec3, 1>), colors, "Vertex colors")),
         ((gal::PolyMesh, outmesh, "Colored mesh with vertex colors.")))
{
  outmesh = mesh;
  outmesh.request_vertex_colors();
  for (TriMesh::VertH vh : outmesh.vertices()) {
    outmesh.set_color(vh, colors[std::min(vh.idx(), int(colors.size() - 1))]);
  }
}

GAL_FUNC(vertexColors,
         "Get the vertex colors of the mesh",
         ((gal::TriMesh, mesh, "Mesh")),
         (((data::WriteView<glm::vec3, 1>), colors, "Vertex colors")))
{
  colors.reserve(mesh.n_vertices());
  std::transform(mesh.vertices_begin(),
                 mesh.vertices_end(),
                 std::back_inserter(colors),
                 [&](gal::TriMesh::VertH vh) { return mesh.color(vh); });
}

GAL_FUNC(decimate,
         "Decimates the mesh while persisting the intermediate meshes",
         ((gal::TriMesh, mesh, "Mesh to be decimated"),
          (int32_t, nCollapses, "Number of edges to collapse.")),
         ((gal::TriMesh, decimated, "The decimated mesh")))
{
  decimated = gal::decimate(mesh, nCollapses);
}

void bind_MeshFunc(py::module& module)
{
  GAL_FN_BIND(centroid, module);
  GAL_FN_BIND(volume, module);
  GAL_FN_BIND(area, module);
  GAL_FN_BIND_TEMPLATE(scale, module, gal::TriMesh);
  GAL_FN_BIND_TEMPLATE(scale, module, gal::PolyMesh);
  GAL_FN_BIND_OVERLOADS(module, bounds, boundsTriMesh, boundsPolyMesh);
  GAL_FN_BIND_OVERLOADS(module, numFaces, numTriMeshFaces, numPolyMeshFaces);
  GAL_FN_BIND(numVertices, module);
  GAL_FN_BIND(vertices, module);
  GAL_FN_BIND_OVERLOADS(module, vertex, vertexTriMesh, vertexPolyMesh);
  GAL_FN_BIND_OVERLOADS(module, halfedge, halfedgeTriMesh, halfedgePolyMesh);
  GAL_FN_BIND(loadTriangleMesh, module);
  GAL_FN_BIND(loadPolyMesh, module);
  GAL_FN_BIND(clipMesh, module);
  GAL_FN_BIND(meshSphereQuery, module);
  GAL_FN_BIND_OVERLOADS(module, subMesh, subTriMesh, subPolyMesh);
  GAL_FN_BIND(closestPoints, module);
  GAL_FN_BIND(rectangleMesh, module);
  GAL_FN_BIND_OVERLOADS(
    module, meshWithVertexColors, triMeshWithVertexColors, polyMeshWithVertexColors);
  GAL_FN_BIND(vertexColors, module);
  GAL_FN_BIND(decimate, module);
}

}  // namespace func
}  // namespace gal

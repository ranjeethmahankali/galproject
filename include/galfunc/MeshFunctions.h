#pragma once

#include <galcore/ObjLoader.h>
#include <galcore/Types.h>
#include <galfunc/GeomFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(meshCentroid,
              1,
              1,
              "Gets the centroid of a mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((glm::vec3, centroid, "x coordinate")));

GAL_FUNC_DECL(meshVolume,
              1,
              1,
              "Gets the volume of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, volume, "Volume of the mesh")));

GAL_FUNC_DECL(meshSurfaceArea,
              1,
              1,
              "Gets the surface area of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, area, "Surface area of the mesh")));

GAL_FUNC_DECL(loadObjFile,
              1,
              1,
              "Loads a mesh from an obj file",
              ((std::string, filepath, "The path to the obj file")),
              ((gal::Mesh, mesh, "Loaded mesh")));

GAL_FUNC_DECL(scaleMesh,
              2,
              1,
              "Scales the mesh. Returns a new instance.",
              ((gal::Mesh, mesh, "Scaled mesh"), (float, scale, "Scale")),
              ((gal::Mesh, scaled, "Input mesh")));

GAL_FUNC_DECL(clipMesh,
              2,
              1,
              "Clips the given mesh with the plane. Returns a new mesh.",
              ((gal::Mesh, mesh, "mesh to clip"),
               (gal::Plane, plane, "Plane to clip with")),
              ((gal::Mesh, clipped, "Clipped mesh")));

GAL_FUNC_DECL(meshSphereQuery,
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
               (int32_t, numFaces, "The number of faces in the query results")));

GAL_FUNC_DECL(closestPointsOnMesh,
              2,
              1,
              "Creates the result point cloud by closest-point-querying the mesh with "
              "the given point cloud",
              ((gal::Mesh, mesh, "Mesh"),
               (gal::PointCloud, inCloud, "Query point cloud")),
              ((gal::PointCloud, outCloud, "Result point cloud")));

GAL_FUNC_DECL(meshBbox,
              1,
              1,
              "Gets the bounding box of the mesh",
              ((gal::Mesh, mesh, "Mesh")),
              ((gal::Box3, bounds, "Bounds of the mesh")));

GAL_FUNC_DECL(rectangleMesh,
              3,
              1,
              "Creates a rectangular mesh",
              ((gal::Plane, plane, "plane"),
               (gal::Box2, bounds, "Bounds"),
               (float, edgeLength, "Approximate edge length.")),
              ((gal::Mesh, mesh, "Resulting rectangular mesh")));

GAL_FUNC_DECL(meshWithVertexColorsFromLambda,
              1,
              1,
              "Assigns mesh vertex colors using the lambda.",
              ((gal::Mesh, inMesh, "Input mesh"),
               (gal::func::store::Lambda,
                colorFn,
                "Subgraph to be used as a lambda to compute vertex colors. Lambda should "
                "have one argument of type point and should output a 3-component vector "
                "representing the color")),
              ((gal::Mesh, outMesh, "Output mesh")));

}  // namespace func
}  // namespace gal

#define GAL_MeshFunctions                                                      \
  meshCentroid, meshVolume, meshSurfaceArea, loadObjFile, scaleMesh, clipMesh, \
    meshSphereQuery, closestPointsOnMesh, meshBbox, rectangleMesh,             \
    meshWithVertexColorsFromLambda

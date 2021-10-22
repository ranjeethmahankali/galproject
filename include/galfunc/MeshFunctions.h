#pragma once

#include <galcore/ObjLoader.h>
#include <galcore/Types.h>
#include <galfunc/GeomFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(meshCentroid,
              "Gets the centroid of a mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((glm::vec3, centroid, "x coordinate")));

GAL_FUNC_DECL(meshVolume,
              "Gets the volume of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, volume, "Volume of the mesh")));

GAL_FUNC_DECL(meshSurfaceArea,
              "Gets the surface area of the mesh",
              ((gal::Mesh, mesh, "The mesh")),
              ((float, area, "Surface area of the mesh")));

GAL_FUNC_DECL(loadObjFile,
              "Loads a mesh from an obj file",
              ((std::string, filepath, "The path to the obj file")),
              ((gal::Mesh, mesh, "Loaded mesh")));

GAL_FUNC_DECL(scaleMesh,
              "Scales the mesh. Returns a new instance.",
              ((gal::Mesh, mesh, "Scaled mesh"), (float, scale, "Scale")),
              ((gal::Mesh, scaled, "Input mesh")));

GAL_FUNC_DECL(clipMesh,
              "Clips the given mesh with the plane. Returns a new mesh.",
              ((gal::Mesh, mesh, "mesh to clip"),
               (gal::Plane, plane, "Plane to clip with")),
              ((gal::Mesh, clipped, "Clipped mesh")));

GAL_FUNC_DECL(meshSphereQuery,
              "Queries the mesh face rtree with the given sphere and "
              "returns the new sub-mesh",
              ((gal::Mesh, mesh, "Mesh to query"),
               (gal::Sphere, sphere, "Sphere to query the faces with")),
              ((gal::Mesh, resultMesh, "Mesh with the queried faces"),
               ((data::WriteView<int32_t, 1>),
                faceIndices,
                "Indices of the faces that are inside / near the query sphere"),
               (int32_t, numFaces, "The number of faces in the query results")));

GAL_FUNC_DECL(closestPointsOnMesh,
              "Creates the result point cloud by closest-point-querying the mesh with "
              "the given point cloud",
              ((gal::Mesh, mesh, "Mesh"),
               ((data::ReadView<glm::vec3, 1>), inCloud, "Query point cloud")),
              (((data::WriteView<glm::vec3, 1>), outCloud, "Result point cloud")));

GAL_FUNC_DECL(meshBbox,
              "Gets the bounding box of the mesh",
              ((gal::Mesh, mesh, "Mesh")),
              ((gal::Box3, bounds, "Bounds of the mesh")));

GAL_FUNC_DECL(rectangleMesh,
              "Creates a rectangular mesh",
              ((gal::Plane, plane, "plane"),
               (gal::Box2, bounds, "Bounds"),
               (float, edgeLength, "Approximate edge length.")),
              ((gal::Mesh, mesh, "Resulting rectangular mesh")));

}  // namespace func
}  // namespace gal

#define GAL_MeshFunctions                                                      \
  meshCentroid, meshVolume, meshSurfaceArea, loadObjFile, scaleMesh, clipMesh, \
    meshSphereQuery, closestPointsOnMesh, meshBbox, rectangleMesh

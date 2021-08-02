#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(vec3,
              3,
              1,
              "Creates a 3D vector from coordinates",
              ((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")),
              ((glm::vec3, vector, "3D vector")));

GAL_FUNC_DECL(vec2,
              2,
              1,
              "Creates a 2D vector from coordinates",
              ((float, x, "x coordinate"), (float, y, "y coordinate")),
              ((glm::vec2, vector, "2D vector")));

GAL_FUNC_DECL(vec3FromVec2,
              1,
              1,
              "Creates a 2D vector from coordinates",
              ((glm::vec2, v2, "2d vector")),
              ((glm::vec3, v3, "3D vector")));

GAL_FUNC_DECL(vec2FromVec3,
              1,
              1,
              "Creates a 2D vector from coordinates",
              ((glm::vec3, v3, "3d vector")),
              ((glm::vec2, v2, "2d vector")));

GAL_FUNC_DECL(vec3Coords,
              1,
              3,
              "Gets the coordinates of the vector",
              ((glm::vec3, v, "Vector")),
              ((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")));

GAL_FUNC_DECL(vec2Coords,
              1,
              3,
              "Gets the coordinates of the vector",
              ((glm::vec2, v, "Vector")),
              ((float, x, "x coordinate"), (float, y, "y coordinate")));

GAL_FUNC_DECL(plane,
              2,
              1,
              "Creates a plane with the given point and normal",
              ((glm::vec3, point, "Point"), (glm::vec3, normal, "Normal")),
              ((gal::Plane, plane, "The plane")));

GAL_FUNC_DECL(box3,
              2,
              1,
              "Creates a 3d box with the two given points",
              ((glm::vec3, min, "min point"), (glm::vec3, max, "max point")),
              ((gal::Box3, box, "Box")));

GAL_FUNC_DECL(box2,
              2,
              1,
              "Creates a 2d box with the two given points",
              ((glm::vec2, min, "min point"), (glm::vec2, max, "max point")),
              ((gal::Box2, box, "Box")));

GAL_FUNC_DECL(randomPointCloudFromBox,
              2,
              1,
              "Creates a random point cloud with points inside the given box",
              ((gal::Box3, box, "Box to sample from"),
               (int32_t, numPoints, "Number of points to sample")),
              ((gal::PointCloud, cloud, "Point cloud")));

GAL_FUNC_DECL(pointCloudConvexHull,
              1,
              1,
              "Creates a convex hull from the given point cloud",
              ((gal::PointCloud, cloud, "Point cloud")),
              ((gal::Mesh, hull, "Convex hull")));

GAL_FUNC_DECL(listVec3FromPointCloud,
              1,
              1,
              "Creates a point list from a point cloud",
              ((gal::PointCloud, cloud, "Point cloud")),
              ((std::vector<glm::vec3>, pts, "Point list")));

GAL_FUNC_DECL(pointCloud3d,
              1,
              1,
              "Creates a point cloud from the list of points",
              ((std::vector<glm::vec3>, points, "points")),
              ((gal::PointCloud, cloud, "Point cloud")));

GAL_FUNC_DECL(distance,
              2,
              1,
              "Gets the distance betwen the two points",
              ((glm::vec3, a, "first point"), (glm::vec3, b, "second point")),
              ((float, dist, "Distance")));

GAL_FUNC_DECL(pointCloudFarthestPt,
              2,
              1,
              "Gets the farthest point in the cloud from the given point",
              ((gal::PointCloud, cloud, "Pointcloud"), (glm::vec3, pt, "point")),
              ((glm::vec3, farthest, "Farthest")));

}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_GeomFunctions                                                             \
  vec3, vec2, vec3FromVec2, vec2FromVec3, plane, box3, box2, randomPointCloudFromBox, \
    pointCloudConvexHull, pointCloud3d, distance, pointCloudFarthestPt,               \
    listVec3FromPointCloud

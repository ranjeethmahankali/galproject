#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(vec3,
              "Creates a 3D vector from coordinates",
              ((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")),
              ((glm::vec3, vector, "3D vector")));

GAL_FUNC_DECL(vec2,
              "Creates a 2D vector from coordinates",
              ((float, x, "x coordinate"), (float, y, "y coordinate")),
              ((glm::vec2, vector, "2D vector")));

GAL_FUNC_DECL(vec3FromVec2,
              "Creates a 2D vector from coordinates",
              ((glm::vec2, v2, "2d vector")),
              ((glm::vec3, v3, "3D vector")));

GAL_FUNC_DECL(vec2FromVec3,
              "Creates a 2D vector from coordinates",
              ((glm::vec3, v3, "3d vector")),
              ((glm::vec2, v2, "2d vector")));

GAL_FUNC_DECL(vec3Coords,
              "Gets the coordinates of the vector",
              ((glm::vec3, v, "Vector")),
              ((float, x, "x coordinate"),
               (float, y, "y coordinate"),
               (float, z, "z coordinate")));

GAL_FUNC_DECL(vec2Coords,
              "Gets the coordinates of the vector",
              ((glm::vec2, v, "Vector")),
              ((float, x, "x coordinate"), (float, y, "y coordinate")));

GAL_FUNC_DECL(plane,
              "Creates a plane with the given point and normal",
              ((glm::vec3, point, "Point"), (glm::vec3, normal, "Normal")),
              ((gal::Plane, plane, "The plane")));

GAL_FUNC_DECL(box3,
              "Creates a 3d box with the two given points",
              ((glm::vec3, min, "min point"), (glm::vec3, max, "max point")),
              ((gal::Box3, box, "Box")));

GAL_FUNC_DECL(box2,
              "Creates a 2d box with the two given points",
              ((glm::vec2, min, "min point"), (glm::vec2, max, "max point")),
              ((gal::Box2, box, "Box")));

GAL_FUNC_DECL(randomPointsInBox,
              "Creates a random point cloud with points inside the given box",
              ((gal::Box3, box, "Box to sample from"),
               (int32_t, numPoints, "Number of points to sample")),
              (((data::WriteView<glm::vec3, 1>), cloud, "Point cloud")));

GAL_FUNC_DECL(convexHullFromPoints,
              "Creates a convex hull from the given point cloud",
              (((data::ReadView<glm::vec3, 1>), cloud, "Point cloud")),
              ((gal::Mesh, hull, "Convex hull")));

GAL_FUNC_DECL(pointCloud3d,
              "Creates a point cloud from the list of points",
              (((data::ReadView<glm::vec3, 1>), points, "points")),
              ((gal::PointCloud, cloud, "Point cloud")));

GAL_FUNC_DECL(distance,
              "Gets the distance betwen the two points",
              ((glm::vec3, a, "first point"), (glm::vec3, b, "second point")),
              ((float, dist, "Distance")));
}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_GeomFunctions                                                       \
  vec3, vec2, vec3FromVec2, vec2FromVec3, plane, box3, box2, randomPointsInBox, \
    convexHullFromPoints, pointCloud3d, distance

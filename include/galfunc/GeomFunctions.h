#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(((glm::vec3, vector, "3D vector")),
              vec3,
              true,
              3,
              "Creates a 3D vector from coordinates",
              (float, x, "x coordinate"),
              (float, y, "y coordinate"),
              (float, z, "z coordinate"));

GAL_FUNC_DECL(((glm::vec2, vector, "2D vector")),
              vec2,
              true,
              2,
              "Creates a 2D vector from coordinates",
              (float, x, "x coordinate"),
              (float, y, "y coordinate"));

GAL_FUNC_DECL(((gal::Sphere, sphere, "Sphere")),
              sphere,
              true,
              2,
              "Creates a new sphere",
              (glm::vec3, center, "Center"),
              (float, radius, "Radius"));

GAL_FUNC_DECL(((gal::Plane, plane, "The plane")),
              plane,
              true,
              2,
              "Creates a plane with the given point and normal",
              (glm::vec3, point, "Point"),
              (glm::vec3, normal, "Normal"));

GAL_FUNC_DECL(((gal::Box3, box, "Box")),
              box3,
              true,
              2,
              "Creates a 3d box with the two given points",
              (glm::vec3, min, "min point"),
              (glm::vec3, max, "max point"));

GAL_FUNC_DECL(((gal::Box2, box, "Box")),
              box2,
              true,
              2,
              "Creates a 2d box with the two given points",
              (glm::vec2, min, "min point"),
              (glm::vec2, max, "max point"));

GAL_FUNC_DECL(((gal::PointCloud, cloud, "Point cloud")),
              randomPointCloudFromBox,
              true,
              2,
              "Creates a random point cloud with points inside the given box",
              (gal::Box3, box, "Box to sample from"),
              (int32_t, numPoints, "Number of points to sample"));

GAL_FUNC_DECL(((gal::Mesh, hull, "Convex hull")),
              pointCloudConvexHull,
              true,
              1,
              "Creates a convex hull from the given point cloud",
              (gal::PointCloud, cloud, "Point cloud"));

GAL_FUNC_DECL(((gal::Circle2d, circle, "Bounding circle")),
              boundingCircle,
              true,
              1,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (gal::PointCloud, points, "Points"));

GAL_FUNC_DECL(((gal::PointCloud, cloud, "Point cloud")),
              pointCloud3d,
              true,
              1,
              "Creates a point cloud from the list of points",
              (std::vector<glm::vec3>, points, "points"));

GAL_FUNC_DECL(((gal::Sphere, sphere, "Bounding sphere")),
              boundingSphere,
              true,
              1,
              "Creates a minimum bounding sphere for the given points.",
              (gal::PointCloud, points, "Points"));

GAL_FUNC_DECL(((gal::Circle2d, circle, "Circle")),
              circle2d,
              true,
              2,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (glm::vec2, center, "Center"),
              (float, radius, "Radius"));

}  // namespace func
}  // namespace gal

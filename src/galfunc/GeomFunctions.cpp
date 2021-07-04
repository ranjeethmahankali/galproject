#include <galcore/ConvexHull.h>
#include <galfunc/GeomFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(((glm::vec3, vector, "3D vector")),
              vec3,
              true,
              3,
              "Creates a 3D vector from coordinates",
              (float, x, "x coordinate"),
              (float, y, "y coordinate"),
              (float, z, "z coordinate"))
{
  return std::make_tuple(std::make_shared<glm::vec3>(*x, *y, *z));
};

GAL_FUNC_DEFN(((glm::vec2, vector, "2D vector")),
              vec2,
              true,
              2,
              "Creates a 2D vector from coordinates",
              (float, x, "x coordinate"),
              (float, y, "y coordinate"))
{
  return std::make_tuple(std::make_shared<glm::vec2>(*x, *y));
};

GAL_FUNC_DEFN(((gal::Sphere, sphere, "Sphere")),
              sphere,
              true,
              2,
              "Creates a new sphere",
              (glm::vec3, center, "Center"),
              (float, radius, "Radius"))
{
  return std::make_tuple(std::make_shared<gal::Sphere>(gal::Sphere {*center, *radius}));
};

GAL_FUNC_DEFN(((gal::Plane, plane, "The plane")),
              plane,
              true,
              2,
              "Creates a plane with the given point and normal",
              (glm::vec3, point, "Point"),
              (glm::vec3, normal, "Normal"))
{
  return std::make_tuple(std::make_shared<gal::Plane>(*point, *normal));
};

GAL_FUNC_DEFN(((gal::Box3, box, "Box")),
              box3,
              true,
              2,
              "Creates a 3d box with the two given points",
              (glm::vec3, min, "min point"),
              (glm::vec3, max, "max point"))
{
  return std::make_tuple(std::make_shared<Box3>(*min, *max));
};

GAL_FUNC_DEFN(((gal::Box2, box, "Box")),
              box2,
              true,
              2,
              "Creates a 2d box with the two given points",
              (glm::vec2, min, "min point"),
              (glm::vec2, max, "max point"))
{
  return std::make_tuple(std::make_shared<gal::Box2>(*min, *max));
};

GAL_FUNC_DEFN(((gal::PointCloud, cloud, "Point cloud")),
              randomPointCloudFromBox,
              true,
              2,
              "Creates a random point cloud with points inside the given box",
              (gal::Box3, box, "Box to sample from"),
              (int32_t, numPoints, "Number of points to sample"))
{
  auto   cloud = std::make_shared<gal::PointCloud>();
  size_t nPts  = size_t(*numPoints);
  cloud->reserve(nPts);
  box->randomPoints(nPts, std::back_inserter(*cloud));
  return std::make_tuple(cloud);
};

GAL_FUNC_DEFN(((gal::Mesh, hull, "Convex hull")),
              pointCloudConvexHull,
              true,
              1,
              "Creates a convex hull from the given point cloud",
              (gal::PointCloud, cloud, "Point cloud"))
{
  gal::ConvexHull hull(cloud->begin(), cloud->end());
  return std::make_tuple(std::make_shared<gal::Mesh>(hull.toMesh()));
};

GAL_FUNC_DEFN(((gal::Circle2d, circle, "Bounding circle")),
              boundingCircle,
              true,
              1,
              "Creates a bounding circle for the given points. The 3d points are "
              "flattened to 2d by removing the z-coordinate.",
              (gal::PointCloud, points, "Points"))
{
  std::vector<glm::vec2> pts2d;
  pts2d.reserve(points->size());
  std::transform(
    points->begin(), points->end(), std::back_inserter(pts2d), [](const glm::vec3& p) {
      return glm::vec2(p);
    });
  return std::make_tuple(
    std::make_shared<gal::Circle2d>(gal::Circle2d::minBoundingCircle(pts2d)));
};

GAL_FUNC_DEFN(((gal::PointCloud, cloud, "Point cloud")),
              pointCloud3d,
              true,
              1,
              "Creates a point cloud from the list of points",
              (std::vector<glm::vec3>, points, "points"))
{
  return std::make_tuple(std::make_shared<gal::PointCloud>(*points));
};

GAL_FUNC_DEFN(((gal::Sphere, sphere, "Bounding sphere")),
              boundingSphere,
              true,
              1,
              "Creates a minimum bounding sphere for the given points.",
              (gal::PointCloud, points, "Points"))
{
  return std::make_tuple(
    std::make_shared<gal::Sphere>(gal::Sphere::minBoundingSphere(*points)));
};

}  // namespace func
}  // namespace gal

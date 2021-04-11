#pragma once

#include <galcore/Box.h>
#include <galcore/Circle2d.h>
#include <galcore/Plane.h>
#include <galcore/PointCloud.h>
#include <galcore/Sphere.h>
#include <galcore/Mesh.h>
#include <galfunc/Functions.h>

GAL_TYPE_INFO(glm::vec3, 0x33821151);
GAL_TYPE_INFO(gal::Sphere, 0x5e8f631c);
GAL_TYPE_INFO(gal::Plane, 0x591f323f);
GAL_TYPE_INFO(gal::Box3, 0x8fcb9e01);
GAL_TYPE_INFO(gal::PointCloud, 0xe6e934eb);
GAL_TYPE_INFO(gal::Circle2d, 0X3271dc29);

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

}  // namespace func
}  // namespace gal
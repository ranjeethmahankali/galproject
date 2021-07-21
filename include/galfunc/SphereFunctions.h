#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(sphere,
              2,
              1,
              "Creates a new sphere",
              ((glm::vec3, center, "Center"), (float, radius, "Radius")),
              ((gal::Sphere, sphere, "Sphere")));

GAL_FUNC_DECL(boundingSphere,
              1,
              3,
              "Creates a minimum bounding sphere for the given points.",
              ((gal::PointCloud, points, "Points")),
              ((gal::Sphere, sphere, "Bounding sphere"),
               (glm::vec3, center, "Center of the sphere"),
               (float, radius, "Radius of the sphere")));

}  // namespace func
}  // namespace gal

#define GAL_SphereFunctions sphere, boundingSphere

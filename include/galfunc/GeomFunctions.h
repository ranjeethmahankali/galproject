#pragma once

#include <galcore/Sphere.h>
#include <galfunc/Functions.h>

GAL_TYPE_INFO(glm::vec3, 0x33821151);
GAL_TYPE_INFO(gal::Sphere, 0x5e8f631c);

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

}  // namespace func
}  // namespace gal
#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(((gal::Sphere, sphere, "Sphere")),
              sphere,
              true,
              2,
              "Creates a new sphere",
              (glm::vec3, center, "Center"),
              (float, radius, "Radius"));

GAL_FUNC_DECL(((gal::Sphere, sphere, "Bounding sphere")),
              boundingSphere,
              true,
              1,
              "Creates a minimum bounding sphere for the given points.",
              (gal::PointCloud, points, "Points"));

}  // namespace func
}  // namespace gal
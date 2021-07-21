#pragma once

#include <glm/glm.hpp>

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(absPath,
              1,
              1,
              "Gets the absolute path given the path relative to the current binary.",
              ((std::string, relpath, "Path relative to the current binary")),
              ((std::string, apath, "Absolute path output")));

GAL_FUNC_DECL(mapValueToColor,
              3,
              1,
              "Maps the given value, w.r.t the given range, to the given color scheme. "
              "And returns the color",
              ((float, value, "value"),
               (glm::vec2, range, "Possible range of values"),
               (std::vector<glm::vec3>, colorScheme, "The scheme of colors")),
              ((glm::vec3, color, "Color")));

}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_UtilFunctions absPath, mapValueToColor

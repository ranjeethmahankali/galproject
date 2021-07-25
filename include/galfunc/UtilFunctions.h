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

GAL_FUNC_DECL(sin,
              1,
              1,
              "Calculates the sine",
              ((float, x, "Value for which to compute sine")),
              ((float, result, "Sine of the input value")));

GAL_FUNC_DECL(cos,
              1,
              1,
              "Calculates the cosine",
              ((float, x, "Value for which to compute cosine")),
              ((float, result, "Cosine of the input value")));

GAL_FUNC_DECL(tan,
              1,
              1,
              "Calculates the tan",
              ((float, x, "Value for which to compute tan")),
              ((float, result, "Tan of the input value")));

GAL_FUNC_DECL(arcsin,
              1,
              1,
              "Calculates the inverse sine",
              ((float, x, "The value for which to compute the inverse sine")),
              ((float, result, "The inverse sine of the input.")));

GAL_FUNC_DECL(arccos,
              1,
              1,
              "Calculates the inverse cosine",
              ((float, x, "The value for which to compute the inverse cosine")),
              ((float, result, "The inverse cosine of the input.")));

GAL_FUNC_DECL(arctan,
              1,
              1,
              "Calculates the inverse tan",
              ((float, x, "The value for which to compute the inverse tan")),
              ((float, result, "The inverse tan of the input.")));

GAL_FUNC_DECL(powf32,
              2,
              1,
              "Raises the base to the power",
              ((float, base, "Base"), (float, power, "Power")),
              ((float, result, "Result")));

}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_UtilFunctions absPath, mapValueToColor

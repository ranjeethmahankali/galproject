#pragma once

#include <glm/glm.hpp>

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(absPath,
              "Gets the absolute path given the path relative to the current binary.",
              ((std::string, relpath, "Path relative to the current binary")),
              ((std::string, apath, "Absolute path output")));

GAL_FUNC_DECL(mapValueToColor,
              "Maps the given value, w.r.t the given range, to the given color scheme. "
              "And returns the color",
              ((float, value, "value"),
               (glm::vec2, range, "Possible range of values"),
               (std::vector<glm::vec3>, colorScheme, "The scheme of colors")),
              ((glm::vec3, color, "Color")));

GAL_FUNC_DECL(sin,
              "Calculates the sine",
              ((float, x, "Value for which to compute sine")),
              ((float, result, "Sine of the input value")));

GAL_FUNC_DECL(cos,
              "Calculates the cosine",
              ((float, x, "Value for which to compute cosine")),
              ((float, result, "Cosine of the input value")));

GAL_FUNC_DECL(tan,
              "Calculates the tan",
              ((float, x, "Value for which to compute tan")),
              ((float, result, "Tan of the input value")));

GAL_FUNC_DECL(arcsin,
              "Calculates the inverse sine",
              ((float, x, "The value for which to compute the inverse sine")),
              ((float, result, "The inverse sine of the input.")));

GAL_FUNC_DECL(arccos,
              "Calculates the inverse cosine",
              ((float, x, "The value for which to compute the inverse cosine")),
              ((float, result, "The inverse cosine of the input.")));

GAL_FUNC_DECL(arctan,
              "Calculates the inverse tan",
              ((float, x, "The value for which to compute the inverse tan")),
              ((float, result, "The inverse tan of the input.")));

GAL_FUNC_DECL(powf32,
              "Raises the base to the power",
              ((float, base, "Base"), (float, power, "Power")),
              ((float, result, "Result")));

GAL_FUNC_DECL(sqrtf32,
              "Square root of the given value",
              ((float, x, "Value for which to compute the square root")),
              ((float, result, "Square root")));

GAL_FUNC_DECL(addf32,
              "Adds two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, sum, "The sum of two numbers")));

GAL_FUNC_DECL(subf32,
              "Subtracts the second number from the first",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, diff, "The difference")));

GAL_FUNC_DECL(mulf32,
              "Multiplies the two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, prod, "The product")));

GAL_FUNC_DECL(divf32,
              "Divides the first number with the second.",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, quot, "The quotient")));

GAL_FUNC_DECL(series,
              "Series of integers",
              ((int32_t, start, "Start"),
               (int32_t, step, "Step"),
               (int32_t, count, "Count")),
              (((data::WriteView<int32_t, 1>), arr, "The series of integers")));

GAL_FUNC_DECL(listSum,
              "Sums a list of integers",
              (((data::ReadView<int32_t, 1>), nums, "Integers to sum")),
              ((int32_t, result, "Sum of the integers")));

GAL_FUNC_DECL(combintations,
              "Combinations from a list of integers",
              (((data::ReadView<int32_t, 1>), items, "Items to create combinations with"),
               (int32_t, count, "The number of items in each combination.")),
              (((data::WriteView<int32_t, 2>), combs, "The combinations as a 2d tree.")));

}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_UtilFunctions                                                           \
  absPath, mapValueToColor, sin, cos, tan, arcsin, arccos, arctan, powf32, sqrtf32, \
    addf32, subf32, mulf32, divf32

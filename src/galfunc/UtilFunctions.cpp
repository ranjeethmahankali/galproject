#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/Functions.h>
#include "galcore/Traits.h"

namespace gal {
namespace func {

GAL_FUNC(absPath,
         "Gets the absolute path given the path relative to the current binary.",
         ((std::string, relpath, "Path relative to the current binary")),
         ((std::string, apath, "Absolute path output")))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC(sin,
         "Calculates the sine",
         ((float, x, "Value for which to compute sine")),
         ((float, result, "Sine of the input value")))
{
  result = std::sin(x);
}

GAL_FUNC(cos,
         "Calculates the cosine",
         ((float, x, "Value for which to compute cosine")),
         ((float, result, "Cosine of the input value")))
{
  result = std::cos(x);
}

GAL_FUNC(tan,
         "Calculates the tan",
         ((float, x, "Value for which to compute tan")),
         ((float, result, "Tan of the input value")))
{
  result = std::tan(x);
}

GAL_FUNC(arcsin,
         "Calculates the inverse sine",
         ((float, x, "The value for which to compute the inverse sine")),
         ((float, result, "The inverse sine of the input.")))
{
  result = std::asin(x);
}

GAL_FUNC(arccos,
         "Calculates the inverse cosine",
         ((float, x, "The value for which to compute the inverse cosine")),
         ((float, result, "The inverse cosine of the input.")))
{
  result = std::acos(x);
}

GAL_FUNC(arctan,
         "Calculates the inverse tan",
         ((float, x, "The value for which to compute the inverse tan")),
         ((float, result, "The inverse tan of the input.")))
{
  result = std::atan(x);
}

GAL_FUNC(powf32,
         "Raises the base to the power",
         ((float, base, "Base"), (float, power, "Power")),
         ((float, result, "Result")))
{
  result = std::pow(base, power);
}

GAL_FUNC(sqrtf32,
         "Square root of the given value",
         ((float, x, "Value for which to compute the square root")),
         ((float, result, "Square root")))
{
  result = std::sqrt(x);
}

GAL_FUNC(addf32,
         "Adds two numbers",
         ((float, a, "First number"), (float, b, "Second number")),
         ((float, sum, "The sum of two numbers")))
{
  sum = a + b;
}

GAL_FUNC(subf32,
         "Subtracts the second number from the first",
         ((float, a, "First number"), (float, b, "Second number")),
         ((float, diff, "The difference")))
{
  diff = a - b;
}

GAL_FUNC(mulf32,
         "Multiplies the two numbers",
         ((float, a, "First number"), (float, b, "Second number")),
         ((float, prod, "The product")))
{
  prod = a * b;
}

GAL_FUNC(divf32,
         "Divides the first number with the second.",
         ((float, a, "First number"), (float, b, "Second number")),
         ((float, quot, "The quotient")))
{
  quot = a / b;
}

void bind_UtilFunctions()
{
  GAL_FN_BIND(absPath);
  GAL_FN_BIND(sin);
  GAL_FN_BIND(cos);
  GAL_FN_BIND(tan);
  GAL_FN_BIND(arcsin);
  GAL_FN_BIND(arccos);
  GAL_FN_BIND(arctan);
  GAL_FN_BIND(powf32);
  GAL_FN_BIND(sqrtf32);

  // TODO: These should be overloads that can handle several types.
  GAL_FN_BIND(addf32);
  GAL_FN_BIND(subf32);
  GAL_FN_BIND(mulf32);
  GAL_FN_BIND(divf32);
}

}  // namespace func
}  // namespace gal

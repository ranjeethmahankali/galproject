#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/Functions.h>
#include "galcore/Traits.h"

namespace gal {
namespace func {

GAL_FUNC_DECL(absPath,
              "Gets the absolute path given the path relative to the current binary.",
              ((std::string, relpath, "Path relative to the current binary")),
              ((std::string, apath, "Absolute path output")))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC_DECL(sin,
              "Calculates the sine",
              ((float, x, "Value for which to compute sine")),
              ((float, result, "Sine of the input value")))
{
  result = std::sin(x);
}

GAL_FUNC_DECL(cos,
              "Calculates the cosine",
              ((float, x, "Value for which to compute cosine")),
              ((float, result, "Cosine of the input value")))
{
  result = std::cos(x);
}

GAL_FUNC_DECL(tan,
              "Calculates the tan",
              ((float, x, "Value for which to compute tan")),
              ((float, result, "Tan of the input value")))
{
  result = std::tan(x);
}

GAL_FUNC_DECL(arcsin,
              "Calculates the inverse sine",
              ((float, x, "The value for which to compute the inverse sine")),
              ((float, result, "The inverse sine of the input.")))
{
  result = std::asin(x);
}

GAL_FUNC_DECL(arccos,
              "Calculates the inverse cosine",
              ((float, x, "The value for which to compute the inverse cosine")),
              ((float, result, "The inverse cosine of the input.")))
{
  result = std::acos(x);
}

GAL_FUNC_DECL(arctan,
              "Calculates the inverse tan",
              ((float, x, "The value for which to compute the inverse tan")),
              ((float, result, "The inverse tan of the input.")))
{
  result = std::atan(x);
}

GAL_FUNC_DECL(powf32,
              "Raises the base to the power",
              ((float, base, "Base"), (float, power, "Power")),
              ((float, result, "Result")))
{
  result = std::pow(base, power);
}

GAL_FUNC_DECL(sqrtf32,
              "Square root of the given value",
              ((float, x, "Value for which to compute the square root")),
              ((float, result, "Square root")))
{
  result = std::sqrt(x);
}

GAL_FUNC_DECL(addf32,
              "Adds two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, sum, "The sum of two numbers")))
{
  sum = a + b;
}

GAL_FUNC_DECL(subf32,
              "Subtracts the second number from the first",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, diff, "The difference")))
{
  diff = a - b;
}

GAL_FUNC_DECL(mulf32,
              "Multiplies the two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, prod, "The product")))
{
  prod = a * b;
}

GAL_FUNC_DECL(divf32,
              "Divides the first number with the second.",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, quot, "The quotient")))
{
  quot = a / b;
}

void bind_UtilFunctions()
{
  bind_absPath();
  bind_sin();
  bind_cos();
  bind_tan();
  bind_arcsin();
  bind_arccos();
  bind_arctan();
  bind_powf32();
  bind_sqrtf32();

  // TODO: These should be overloads that can handle several types.
  bind_addf32();
  bind_subf32();
  bind_mulf32();
  bind_divf32();
}

}  // namespace func
}  // namespace gal

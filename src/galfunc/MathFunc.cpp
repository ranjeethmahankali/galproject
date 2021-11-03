#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeHelper.h>

namespace gal {
namespace func {

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

GAL_FUNC_TEMPLATE(((typename, T)),
                  add,
                  "Adds two numbers",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, sum, "The sum of two numbers")))
{
  sum = a + b;
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  sub,
                  "Subtracts the second number from the first",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, diff, "The difference")))
{
  diff = a - b;
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  mul,
                  "Multiplies the two numbers",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, prod, "The product")))
{
  prod = a * b;
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  div,
                  "Divides the first number with the second.",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, quot, "The quotient")))
{
  quot = a / b;
}

void bind_MathFunctions()
{
  GAL_FN_BIND(sin, cos, tan, arcsin, arccos, arctan, powf32, sqrtf32);

  GAL_FN_BIND_TEMPLATE(add, float);
  GAL_FN_BIND_TEMPLATE(add, int32_t);
  GAL_FN_BIND_TEMPLATE(sub, float);
  GAL_FN_BIND_TEMPLATE(sub, int32_t);
  GAL_FN_BIND_TEMPLATE(mul, float);
  GAL_FN_BIND_TEMPLATE(mul, int32_t);
  GAL_FN_BIND_TEMPLATE(div, float);
  GAL_FN_BIND_TEMPLATE(div, int32_t);
}

}  // namespace func
}  // namespace gal

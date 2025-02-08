#include <Functions.h>
#include <TypeManager.h>
#include <Util.h>

namespace gal {
namespace func {

GAL_FUNC(sin,  // NOLINT
         "Calculates the sine",
         ((float, x, "Value for which to compute sine")),
         ((float, result, "Sine of the input value")))
{
  result = std::sin(x);
}

GAL_FUNC(cos,  // NOLINT
         "Calculates the cosine",
         ((float, x, "Value for which to compute cosine")),
         ((float, result, "Cosine of the input value")))
{
  result = std::cos(x);
}

GAL_FUNC(tan,  // NOLINT
         "Calculates the tan",
         ((float, x, "Value for which to compute tan")),
         ((float, result, "Tan of the input value")))
{
  result = std::tan(x);
}

GAL_FUNC(arcsin,  // NOLINT
         "Calculates the inverse sine",
         ((float, x, "The value for which to compute the inverse sine")),
         ((float, result, "The inverse sine of the input.")))
{
  result = std::asin(x);
}

GAL_FUNC(arccos,  // NOLINT
         "Calculates the inverse cosine",
         ((float, x, "The value for which to compute the inverse cosine")),
         ((float, result, "The inverse cosine of the input.")))
{
  result = std::acos(x);
}

GAL_FUNC(arctan,  // NOLINT
         "Calculates the inverse tan",
         ((float, x, "The value for which to compute the inverse tan")),
         ((float, result, "The inverse tan of the input.")))
{
  result = std::atan(x);
}

GAL_FUNC(powf32,  // NOLINT
         "Raises the base to the power",
         ((float, base, "Base"), (float, power, "Power")),
         ((float, result, "Result")))
{
  result = std::pow(base, power);
}

GAL_FUNC(sqrtf32,  // NOLINT
         "Square root of the given value",
         ((float, x, "Value for which to compute the square root")),
         ((float, result, "Square root")))
{
  result = std::sqrt(x);
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  add,
                  "Adds two numbers",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, sum, "The sum of two numbers")))
{
  sum = a + b;
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  sub,
                  "Subtracts the second number from the first",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, diff, "The difference")))
{
  diff = a - b;
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  mul,
                  "Multiplies the two numbers",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, prod, "The product")))
{
  prod = a * b;
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  div,
                  "Divides the first number with the second.",
                  ((T, a, "First number"), (T, b, "Second number")),
                  ((T, quot, "The quotient")))
{
  quot = a / b;
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  max,
                  "Get the max of the two arguments",
                  ((T, a, "First arg"), (T, b, "Second arg")),
                  ((T, out, "Maximum of the input args")))
{
  out = std::max(a, b);
}

GAL_FUNC_TEMPLATE(((typename, T)),  // NOLINT
                  min,
                  "Get the max of the two arguments",
                  ((T, a, "First arg"), (T, b, "Second arg")),
                  ((T, out, "Maximum of the input args")))
{
  out = std::min(a, b);
}

void bind_MathFunc(py::module& mod)
{
  GAL_FN_BIND(sin, mod);
  GAL_FN_BIND(cos, mod);
  GAL_FN_BIND(tan, mod);
  GAL_FN_BIND(arcsin, mod);
  GAL_FN_BIND(arccos, mod);
  GAL_FN_BIND(arctan, mod);
  GAL_FN_BIND(powf32, mod);
  GAL_FN_BIND(sqrtf32, mod);

  GAL_FN_BIND_TEMPLATE(add, mod, float);
  GAL_FN_BIND_TEMPLATE(add, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(sub, mod, float);
  GAL_FN_BIND_TEMPLATE(sub, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(mul, mod, float);
  GAL_FN_BIND_TEMPLATE(mul, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(div, mod, float);
  GAL_FN_BIND_TEMPLATE(div, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(max, mod, float);
  GAL_FN_BIND_TEMPLATE(max, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(min, mod, float);
  GAL_FN_BIND_TEMPLATE(min, mod, int32_t);
}

}  // namespace func
}  // namespace gal

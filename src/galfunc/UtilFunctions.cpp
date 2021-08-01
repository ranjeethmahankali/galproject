#include <galcore/Util.h>
#include <galfunc/UtilFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(absPath,
              1,
              1,
              "Gets the absolute path given the path relative to the current binary.",
              ((std::string, relpath, "Path relative to the current binary")),
              ((std::string, apath, "Absolute path output")))
{
  *apath = gal::utils::absPath(*relpath);
}

GAL_FUNC_DEFN(mapValueToColor,
              3,
              1,
              "Maps the given value, w.r.t the given range, to the given color scheme. "
              "And returns the color",
              ((float, value, "value"),
               (glm::vec2, range, "Possible range of values"),
               (std::vector<glm::vec3>, colorScheme, "The scheme of colors")),
              ((glm::vec3, color, "Color")))
{
  const auto& val = *value;
  const auto& rng = *range;
  float       r   = float(colorScheme->size() - 1) *
            std::clamp((val - rng[0]) / (rng[1] - rng[0]), 0.f, 1.f);

  float  fi = std::floor(r);
  size_t i  = size_t(fi);
  size_t j  = size_t(std::ceil(r));
  r -= fi;

  *color = colorScheme->at(i) * (1.f - r) + colorScheme->at(j) * r;
}

GAL_FUNC_DEFN(sin,
              1,
              1,
              "Calculates the sine",
              ((float, x, "Value for which to compute sine")),
              ((float, result, "Sine of the input value")))
{
  *result = std::sin(*x);
}

GAL_FUNC_DEFN(cos,
              1,
              1,
              "Calculates the cosine",
              ((float, x, "Value for which to compute cosine")),
              ((float, result, "Cosine of the input value")))
{
  *result = std::cos(*x);
}

GAL_FUNC_DEFN(tan,
              1,
              1,
              "Calculates the tan",
              ((float, x, "Value for which to compute tan")),
              ((float, result, "Tan of the input value")))
{
  *result = std::tan(*x);
}

GAL_FUNC_DEFN(arcsin,
              1,
              1,
              "Calculates the inverse sine",
              ((float, x, "The value for which to compute the inverse sine")),
              ((float, result, "The inverse sine of the input.")))
{
  *result = std::asin(*x);
}

GAL_FUNC_DEFN(arccos,
              1,
              1,
              "Calculates the inverse cosine",
              ((float, x, "The value for which to compute the inverse cosine")),
              ((float, result, "The inverse cosine of the input.")))
{
  *result = std::acos(*x);
}

GAL_FUNC_DEFN(arctan,
              1,
              1,
              "Calculates the inverse tan",
              ((float, x, "The value for which to compute the inverse tan")),
              ((float, result, "The inverse tan of the input.")))
{
  *result = std::atan(*x);
}

GAL_FUNC_DEFN(powf32,
              2,
              1,
              "Raises the base to the power",
              ((float, base, "Base"), (float, power, "Power")),
              ((float, result, "Result")))
{
  *result = std::pow(*base, *power);
}

GAL_FUNC_DEFN(sqrtf32,
              1,
              1,
              "Square root of the given value",
              ((float, x, "Value for which to compute the square root")),
              ((float, result, "Square root")))
{
  *result = std::sqrt(*x);
}

GAL_FUNC_DEFN(addf32,
              2,
              1,
              "Adds two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, sum, "The sum of two numbers")))
{
  *sum = *a + *b;
}

GAL_FUNC_DEFN(subf32,
              2,
              1,
              "Subtracts the second number from the first",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, diff, "The difference")))
{
  *diff = *a - *b;
}

GAL_FUNC_DEFN(mulf32,
              2,
              1,
              "Multiplies the two numbers",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, prod, "The product")))
{
  *prod = (*a) * (*b);
}

GAL_FUNC_DEFN(divf32,
              2,
              1,
              "Divides the first number with the second.",
              ((float, a, "First number"), (float, b, "Second number")),
              ((float, quot, "The quotient")))
{
  *quot = (*a) / (*b);
}

}  // namespace func
}  // namespace gal

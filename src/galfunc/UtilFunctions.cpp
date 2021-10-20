#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/UtilFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(absPath, ((std::string, relpath)), ((std::string, apath)))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC_DEFN(sin, ((float, x)), ((float, result)))
{
  result = std::sin(x);
}

GAL_FUNC_DEFN(cos, ((float, x)), ((float, result)))
{
  result = std::cos(x);
}

GAL_FUNC_DEFN(tan, ((float, x)), ((float, result)))
{
  result = std::tan(x);
}

GAL_FUNC_DEFN(arcsin, ((float, x)), ((float, result)))
{
  result = std::asin(x);
}

GAL_FUNC_DEFN(arccos, ((float, x)), ((float, result)))
{
  result = std::acos(x);
}

GAL_FUNC_DEFN(arctan, ((float, x)), ((float, result)))
{
  result = std::atan(x);
}

GAL_FUNC_DEFN(powf32, ((float, base), (float, power)), ((float, result)))
{
  result = std::pow(base, power);
}

GAL_FUNC_DEFN(sqrtf32, ((float, x)), ((float, result)))
{
  result = std::sqrt(x);
}

GAL_FUNC_DEFN(addf32, ((float, a), (float, b)), ((float, sum)))
{
  sum = a + b;
}

GAL_FUNC_DEFN(subf32, ((float, a), (float, b)), ((float, diff)))
{
  diff = a - b;
}

GAL_FUNC_DEFN(mulf32, ((float, a), (float, b)), ((float, prod)))
{
  prod = a * b;
}

GAL_FUNC_DEFN(divf32, ((float, a), (float, b)), ((float, quot)))
{
  quot = a / b;
}

}  // namespace func
}  // namespace gal

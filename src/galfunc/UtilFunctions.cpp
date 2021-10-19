#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/UtilFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(absPath, ((std::string, relpath)), ((std::string, apath)))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC_DEFN(mapValueToColor,
              ((float, value), (glm::vec2, range), (std::vector<glm::vec3>, colorScheme)),
              ((glm::vec3, color)))
{
  float r = float(colorScheme.size() - 1) *
            std::clamp((value - range[0]) / (range[1] - range[0]), 0.f, 1.f);

  float  fi = std::floor(r);
  size_t i  = size_t(fi);
  size_t j  = size_t(std::ceil(r));
  r -= fi;

  color = colorScheme.at(i) * (1.f - r) + colorScheme.at(j) * r;
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

GAL_FUNC_DEFN(series,
              ((int32_t, start), (int32_t, step), (int32_t, count)),
              (((data::WriteView<int32_t, 1>), arr)))
{
  for (int32_t i = 0, val = start; i < count; i++, val += step) {
    arr.push_back(val);
  }
}

GAL_FUNC_DEFN(listSum, (((data::ReadView<int32_t, 1>), nums)), ((int32_t, result)))
{
  result = 0;
  for (int i : nums) {
    result += i;
  }
  // result = std::accumulate(nums.begin(), nums.end(), 0, std::plus<int> {});
}

GAL_FUNC_DEFN(combinations,
              (((data::ReadView<int32_t, 1>), items), (int32_t, count)),
              (((data::WriteView<int32_t, 2>), combs)))
{
  size_t k = size_t(count);
  combs.reserve(gal::utils::numCombinations(items.size(), k));
  std::vector<int32_t> comb(k);
  gal::utils::combinations(
    k, items.begin(), items.end(), comb.begin(), [&comb, &combs]() {
      auto out = combs.child();
      std::copy(comb.begin(), comb.end(), std::back_inserter(out));
    });
}

}  // namespace func
}  // namespace gal

#include <algorithm>
#include <stdexcept>

#include <Data.h>
#include <Functions.h>
#include <TypeManager.h>
#include <Util.h>

namespace gal {
namespace func {

GAL_FUNC(absPath,
         "Gets the absolute path given the path relative to the current binary.",
         ((std::string, relpath, "Path relative to the current binary")),
         ((std::string, apath, "Absolute path output")))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  toString,
                  "Converts the input data to string, if such a conversion is supported.",
                  ((T, src, "The source data")),
                  ((std::string, result, "String result")))
{
  result = std::to_string(src);
}

GAL_FUNC_TEMPLATE(
  ((typename, T)),
  combinations,
  "Creates all possible combinations of elements from the given list",
  (((data::ReadView<T, 1>), items, "Items to create the combinations from."),
   (int32_t, nc, "Number of items in each combination")),
  (((data::WriteView<T, 2>), combs, "Resulting combinations")))
{
  size_t n = items.size();
  size_t k = size_t(nc);
  combs.reserve(k * utils::numCombinations(items.size(), size_t(k)));
  std::vector<T> temp(k);
  utils::combinations(k, items.begin(), items.end(), temp.begin(), [&]() {
    auto child = combs.child();
    std::move(temp.begin(), temp.end(), std::back_inserter(child));
  });
}

GAL_FUNC(mapValueToColor,
         "Maps the given value w.r.t to the range to a color according to the provided "
         "color scheme",
         ((float, val, "The value to be mapped"),
          (glm::vec2, range, "Limits of the value"),
          ((data::ReadView<glm::vec3, 1>), colorScheme, "Color scheme")),
         ((glm::vec3, color, "Mapped color")))
{
  if (colorScheme.empty()) {
    color = {0.f, 0.f, 0.f};
    return;
  }

  float r = float(colorScheme.size() - 1) *
            std::clamp((val - range[0]) / (range[1] - range[0]), 0.f, 1.f);
  float  fr = std::floor(r);
  size_t i  = size_t(fr);
  size_t j  = size_t(std::ceil(r));
  r -= fr;

  color = colorScheme[i] * (1.f - r) + colorScheme[j] * r;
}

namespace utilfunc {  // Namespace to avoid linker confusion.

template<typename T>
struct bindForAllTypes
{
  static void invoke(py::module& module)
  {
    GAL_FN_BIND_TEMPLATE(combinations, module, T);
  }
};

}  // namespace utilfunc

void bind_UtilFunc(py::module& module)
{
  GAL_FN_BIND(absPath, module);
  GAL_FN_BIND(mapValueToColor, module);
  GAL_FN_BIND_TEMPLATE(toString, module, float);
  GAL_FN_BIND_TEMPLATE(toString, module, int32_t);

  typemanager::invoke<utilfunc::bindForAllTypes>((py::module&)module);
}

}  // namespace func
}  // namespace gal

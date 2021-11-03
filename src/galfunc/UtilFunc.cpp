#include <stdexcept>

#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeManager.h>

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

namespace utilfunc {  // Namespace to avoid linker confusion.

template<typename T>
struct bindAllTypes
{
  static void invoke() { GAL_FN_BIND_TEMPLATE(combinations, T); }
};

}  // namespace utilfunc

void bind_UtilFunc()
{
  GAL_FN_BIND(absPath);
  GAL_FN_BIND_TEMPLATE(toString, float);
  GAL_FN_BIND_TEMPLATE(toString, int32_t);

  typemanager::invoke<utilfunc::bindAllTypes>();
}

}  // namespace func
}  // namespace gal

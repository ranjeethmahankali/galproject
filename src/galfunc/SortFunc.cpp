#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeManager.h>

namespace gal {
namespace func {

GAL_FUNC_TEMPLATE(((typename, TVal), (typename, TKey)),
                  sort,
                  "Sorts a list based on given keys.",
                  (((data::ReadView<TVal, 1>), list, "List to be sorted"),
                   ((data::ReadView<TKey, 1>), keys, "Keys to be used for sorting")),
                  (((data::WriteView<TVal, 1>), sorted, "Sorted values")))
{
  std::vector<size_t> indices(list.size());
  std::iota(indices.begin(), indices.end(), size_t(0));
  std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
    return keys[a] < keys[b];
  });
  sorted.reserve(list.size());
  std::transform(
    indices.begin(), indices.end(), std::back_inserter(sorted), [&](size_t i) {
      return list[i];
    });
}

namespace sortfunc {  // Namespace to avoid linker confusion.

template<typename T>
struct bindAllTypes
{
  static void invoke()
  {
    // Two overloads for the sort function with different key types.
    GAL_FN_BIND_TEMPLATE(sort, T, int32_t);
    GAL_FN_BIND_TEMPLATE(sort, T, float);
  }
};

}  // namespace sortfunc
void bind_SortFunc()
{
  typemanager::invoke<sortfunc::bindAllTypes>();
}

}  // namespace func
}  // namespace gal

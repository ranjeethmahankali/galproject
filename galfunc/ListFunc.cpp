#include <galfunc/Functions.h>
#include <galfunc/TypeManager.h>

namespace gal {
namespace func {

GAL_FUNC_TEMPLATE(((typename, T)),
                  series,
                  "Populates a list with members of an arithmetic progression.",
                  ((T, start, "Start of the series"),
                   (T, step, "Difference between consecutive members of the series."),
                   (int32_t, count, "Number of elements in the series")),
                  (((data::WriteView<T, 1>), result, "The series")))
{
  if (count == 0) {
    return;
  }
  result.resize(count);
  T item = start;
  for (int32_t i = 0; i < count; i++) {
    result[i] = item;
    item += step;
  }
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  repeat,
                  "Creates a list by repeating the given value the given number of times",
                  ((T, val, "The value to be repeated"),
                   (int32_t, count, "Number of times to repeat the value.")),
                  (((data::WriteView<T, 1>), result, "Resulting list")))
{
  result.resize(count);
  for (int32_t i = 0; i < count; i++) {
    result[i] = val;
  }
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  listItem,
                  "Gets an item from the list",
                  (((data::ReadView<T, 1>), list, "List"), (int32_t, index, "Index")),
                  ((T, item, "Item at the index")))
{
  item = list[index];
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  subList,
                  "Gets a slice of the list",
                  (((data::ReadView<T, 1>), list, "Source list"),
                   (int32_t, start, "Index to start copying from"),
                   (int32_t, stop, "Index to copy until")),
                  (((data::WriteView<T, 1>), sublist, "The sub list.")))
{
  if (stop < start || start < 0 || stop >= list.size()) {
    throw std::range_error("Invalid indices for sub-list");
  }
  sublist.resize(stop - start);
  for (int i = start, j = 0; i < stop; i++, j++) {
    sublist[j] = list[i];
  }
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  listSum,
                  "Sum of all items in the list",
                  (((data::ReadView<T, 1>), list, "List")),
                  ((T, sum, "Sum of items")))
{
  static constexpr T sZero = T(0);

  sum = std::accumulate(list.begin(), list.end(), sZero);
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  listLength,
                  "Length of a list",
                  (((data::ReadView<T, 1>), list, "List")),
                  ((int32_t, length, "size of the list")))
{
  length = int32_t(list.size());
}

GAL_FUNC_TEMPLATE(
  ((typename, T)),
  dispatch,
  "Dispatches elements of a list based on a boolean pattern",
  (((data::ReadView<T, 1>), items, "Items to be dispatched"),
   ((data::ReadView<Bool, 1>), pattern, "Boolean values used to dispatch")),
  (((data::WriteView<T, 1>), trueLst, "Items with true values"),
   ((data::WriteView<T, 1>), falseLst, "Items with false values")))
{
  if (items.size() != pattern.size()) {
    throw std::length_error(
      "The dispatch pattern must be of the same length as the list of items to be "
      "dispatched.");
  }
  for (size_t i = 0; i < pattern.size(); i++) {
    if (pattern[i]) {
      trueLst.push_back(items[i]);
    }
    else {
      falseLst.push_back(items[i]);
    }
  }
}

namespace listfunc {  // Anon namespace to avoid linker confusion.

template<typename T>
struct bindForAllTypes
{
  static void invoke(py::module& mod)
  {
    GAL_FN_BIND_TEMPLATE(repeat, mod, T);
    GAL_FN_BIND_TEMPLATE(listItem, mod, T);
    GAL_FN_BIND_TEMPLATE(subList, mod, T);
    GAL_FN_BIND_TEMPLATE(listLength, mod, T);
    GAL_FN_BIND_TEMPLATE(dispatch, mod, T);
  }
};

}  // namespace listfunc
void bind_ListFunc(py::module& mod)
{
  GAL_FN_BIND_TEMPLATE(series, mod, float);
  GAL_FN_BIND_TEMPLATE(series, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(listSum, mod, float);
  GAL_FN_BIND_TEMPLATE(listSum, mod, int32_t);
  typemanager::invoke<listfunc::bindForAllTypes>((py::module&)mod);
}

}  // namespace func
}  // namespace gal

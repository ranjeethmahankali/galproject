#include <stdexcept>

#include <galcore/Traits.h>
#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeHelper.h>

namespace gal {
namespace func {

GAL_FUNC(absPath,
         "Gets the absolute path given the path relative to the current binary.",
         ((std::string, relpath, "Path relative to the current binary")),
         ((std::string, apath, "Absolute path output")))
{
  apath = gal::utils::absPath(relpath);
}

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

template<typename T>
struct bindAllTypes
{
  static void invoke()
  {
    GAL_FN_BIND_TEMPLATE(repeat, T);
    GAL_FN_BIND_TEMPLATE(listItem, T);
    GAL_FN_BIND_TEMPLATE(subList, T);
    GAL_FN_BIND_TEMPLATE(listLength, T);
    GAL_FN_BIND_TEMPLATE(dispatch, T);
    GAL_FN_BIND_TEMPLATE(combinations, T);
  }
};

void bind_UtilFunctions()
{
  GAL_FN_BIND(absPath, sin, cos, tan, arcsin, arccos, arctan, powf32, sqrtf32);

  GAL_FN_BIND_TEMPLATE(add, float);
  GAL_FN_BIND_TEMPLATE(add, int32_t);
  GAL_FN_BIND_TEMPLATE(sub, float);
  GAL_FN_BIND_TEMPLATE(sub, int32_t);
  GAL_FN_BIND_TEMPLATE(mul, float);
  GAL_FN_BIND_TEMPLATE(mul, int32_t);
  GAL_FN_BIND_TEMPLATE(div, float);
  GAL_FN_BIND_TEMPLATE(div, int32_t);
  GAL_FN_BIND_TEMPLATE(series, float);
  GAL_FN_BIND_TEMPLATE(series, int32_t);
  GAL_FN_BIND_TEMPLATE(listSum, float);
  GAL_FN_BIND_TEMPLATE(listSum, int32_t);
  GAL_FN_BIND_TEMPLATE(toString, float);
  GAL_FN_BIND_TEMPLATE(toString, int32_t);

  typemanager::invoke<bindAllTypes>();
}

}  // namespace func
}  // namespace gal

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_TEMPLATE(((typename, T)),
                  treeSum,
                  "Gets the sum of all elements in the tree",
                  ((data::Tree<T>, tree, "Tree to be summed.")),
                  ((T, sum, "Sum of all elements of the tree.")))
{
  static constexpr T sZero = T(0);
  sum = std::accumulate(tree.values().begin(), tree.values().end(), sZero);
}

void bind_TreeFunctions()
{
  GAL_FN_BIND_TEMPLATE(treeSum, int32_t);
  GAL_FN_BIND_TEMPLATE(treeSum, float);
}

}  // namespace func
}  // namespace gal

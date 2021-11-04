#include <galfunc/Functions.h>
#include <galfunc/TypeManager.h>

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

GAL_FUNC_TEMPLATE(((typename, T)),
                  graft,
                  "Grafts the tree data",
                  ((data::Tree<T>, treeIn, "Input tree")),
                  ((data::Tree<T>, treeOut, "Output tree")))
{
  treeOut = treeIn;
  for (auto& d : treeOut.depths()) {
    d++;
  }
}

namespace treefunc {
template<typename T>
struct bindAllTypes
{
  static void invoke() { GAL_FN_BIND_TEMPLATE(graft, T); }
};
}  // namespace treefunc

void bind_TreeFunc()
{
  GAL_FN_BIND_TEMPLATE(treeSum, int32_t);
  GAL_FN_BIND_TEMPLATE(treeSum, float);

  typemanager::invoke<treefunc::bindAllTypes>();
}

}  // namespace func
}  // namespace gal

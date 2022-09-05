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
                  ((data::Tree<T>, treeOut, "Grafted tree")))
{
  treeOut = treeIn;
  treeOut.graft();
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  flatten,
                  "Flattens the tree",
                  ((data::Tree<T>, in, "Input tree")),
                  ((data::Tree<T>, out, "Flattened tree")))
{
  out = in;
  out.flatten();
}

namespace treefunc {
template<typename T>
struct bindForAllTypes
{
  static void invoke(py::module& mod)
  {
    GAL_FN_BIND_TEMPLATE(graft, mod, T);
    GAL_FN_BIND_TEMPLATE(flatten, mod, T);
  }
};
}  // namespace treefunc

void bind_TreeFunc(py::module& mod)
{
  GAL_FN_BIND_TEMPLATE(treeSum, mod, int32_t);
  GAL_FN_BIND_TEMPLATE(treeSum, mod, float);
  typemanager::invoke<treefunc::bindForAllTypes>((py::module&)mod);
}

}  // namespace func
}  // namespace gal

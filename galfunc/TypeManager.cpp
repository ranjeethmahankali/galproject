#include <TypeManager.h>

#define GAL_TEMPLATE_INST(T)                                                  \
  template class gal::func::data::Tree<T>;                                    \
  template struct gal::func::data::ReadView<T, 1>;                            \
  template struct gal::func::data::ReadView<T, 2>;                            \
  template struct gal::func::data::WriteView<T, 1>;                           \
  template struct gal::func::data::WriteView<T, 2>;                           \
  template struct gal::func::data::repeat::CombiView<T, true>;                \
  template struct gal::func::data::repeat::CombiView<T, false>;               \
  template struct gal::func::Converter<gal::func::data::Tree<T>, py::object>; \
  template struct gal::func::Converter<py::object, gal::func::data::Tree<T>>; \
  template struct gal::func::Converter<py::object, T>;                        \
  template struct gal::func::Converter<T, py::object>;                        \
  template struct gal::func::Converter<py::list, std::vector<T>>;             \
  template struct gal::func::Converter<std::vector<T>, py::list>;

MAP(GAL_TEMPLATE_INST, GAL_MANAGED_TYPES)

namespace gal {
namespace func {}
}  // namespace gal

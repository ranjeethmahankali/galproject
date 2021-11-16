#include <galfunc/TypeManager.h>

#define GAL_TEMPLATE_INST(T)                                                             \
  template class gal::func::data::Tree<T>;                                               \
  template struct gal::func::data::ReadView<T, 1>;                                       \
  template struct gal::func::data::ReadView<T, 2>;                                       \
  template struct gal::func::data::WriteView<T, 1>;                                      \
  template struct gal::func::data::WriteView<T, 2>;                                      \
  template struct gal::func::data::repeat::CombiView<T, true>;                           \
  template struct gal::func::data::repeat::CombiView<T, false>;                          \
  template struct gal::func::Converter<gal::func::data::Tree<T>, boost::python::object>; \
  template struct gal::func::Converter<boost::python::object, gal::func::data::Tree<T>>; \
  template struct gal::func::Converter<boost::python::api::const_object_item, T>;        \
  template struct gal::func::Converter<boost::python::object, T>;                        \
  template struct gal::func::Converter<T, boost::python::object>;                        \
  template struct gal::func::Converter<boost::python::list, std::vector<T>>;             \
  template struct gal::func::Converter<std::vector<T>, boost::python::list>;

MAP(GAL_TEMPLATE_INST, GAL_MANAGED_TYPES)

namespace gal {
namespace func {

}
}  // namespace gal

#include <galfunc/TypeManager.h>

#define GAL_TEMPLATE_INST(type)                                   \
  template class gal::func::data::Tree<type>;                     \
  template struct gal::func::data::ReadView<type, 1>;             \
  template struct gal::func::data::ReadView<type, 2>;             \
  template struct gal::func::data::WriteView<type, 1>;            \
  template struct gal::func::data::WriteView<type, 2>;            \
  template struct gal::func::data::repeat::CombiView<type, true>; \
  template struct gal::func::data::repeat::CombiView<type, false>;

MAP(GAL_TEMPLATE_INST, GAL_MANAGED_TYPES)

namespace gal {
namespace func {

}
}  // namespace gal

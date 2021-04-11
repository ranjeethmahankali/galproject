#include <galview/GuiFunctions.h>

namespace gal {
namespace viewfunc {

static view::Panel* sInputPanel  = nullptr;
static view::Panel* sOutputPanel = nullptr;

void initPanels(view::Panel& inputs, view::Panel& outputs)
{
  sInputPanel  = &inputs;
  sOutputPanel = &outputs;
};

}  // namespace viewfunc
}  // namespace gal

BOOST_PYTHON_MODULE(pygalview) {
  // Bindings related to gui
  // Views for drawables
  // Labels for strings
  // Sliders for float input
  // Text fields for string inputs
};
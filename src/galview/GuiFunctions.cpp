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

view::Panel& inputPanel()
{
  return *sInputPanel;
};

view::Panel& outputPanel()
{
  return *sOutputPanel;
};

}  // namespace viewfunc
}  // namespace gal

BOOST_PYTHON_MODULE(pygalview)
{
  // Bindings related to gui
  // Views for drawables
  // Labels for strings
  // Sliders for float input
  // Text fields for string inputs
  def("SliderFloat", gal::viewfunc::pySliderFn<float>);
  def("SliderInt", gal::viewfunc::pySliderFn<int32_t>);
};
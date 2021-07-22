#pragma once
#include <galfunc/Functions.h>
#include <galfunc/Variable.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

void         initPanels(view::Panel& inputs, view::Panel& outputs);
view::Panel& inputPanel();
view::Panel& outputPanel();
void         evalOutputs();

template<typename T>
struct SliderFunc : public gal::func::TVariable<T, T>, public gal::view::Slider<T>
{
public:
  SliderFunc(const std::string& label, T min, T max, T value)
      : gal::func::TVariable<T, T>(value)
      , gal::view::Slider<T>(label, min, max, value) {};

private:
  using gal::view::Slider<T>::addHandler;

protected:
  void handleChanges() override
  {
    if (this->isEdited())
      this->set(this->mValue);

    this->clearEdited();
  };
};

template<typename T>
boost::python::tuple py_slider(const std::string& label,
                               const T&           min,
                               const T&           max,
                               const T&           value)
{
  auto fn = gal::func::store::makeFunction<SliderFunc<T>>(label, min, max, value);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(gal::func::types::makeOutputTuple<1>(*fn));
};

}  // namespace viewfunc
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();

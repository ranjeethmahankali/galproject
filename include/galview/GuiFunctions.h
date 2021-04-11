#pragma once
#include <galfunc/Functions.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

void         initPanels(view::Panel& inputs, view::Panel& outputs);
view::Panel& inputPanel();
view::Panel& outputPanel();

template<typename T>
class Slider : public gal::func::TVariable<T>, public gal::view::Slider<T>
{
public:
  Slider(const std::string& label, T min, T max, T value)
      : gal::func::TVariable<T>(value)
      , gal::view::Slider<T>(label, min, max, value) {};

protected:
  void handleChanges() override
  {
    gal::view::Slider<T>::handleChanges();
    this->set(this->mValue);
  };
};

template<typename T>
gal::func::types::OutputTuple<1> sliderFn(const std::string& label,
                                          const T&           min,
                                          const T&           max,
                                          const T&           value)
{
  auto fn = gal::func::store::makeFunction<Slider<T>>(label, min, max, value);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::types::makeOutputTuple<1>(*fn);
};

template<typename T>
boost::python::tuple pySliderFn(const std::string& label,
                                const T&           min,
                                const T&           max,
                                const T&           value)
{
  return gal::func::pythonRegisterTuple(sliderFn<T>(label, min, max, value));
};

}  // namespace viewfunc
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();
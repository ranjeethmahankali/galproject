#pragma once

#include <glm/glm.hpp>

#include <galfunc/Functions.h>
#include <galfunc/Variable.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

void         initPanels(view::Panel& inputs, view::Panel& outputs);
view::Panel& inputPanel();
view::Panel& outputPanel();
void         evalOutputs();

/**
 * @brief Clears all output registers.
 * This is meant to be called when unloading a demo.
 */
void unloadAllOutputs();

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

template<int N, typename T, glm::qualifier Q>
struct SliderFunc<glm::vec<N, T, Q>>
    : public gal::func::TVariable<glm::vec<N, T, Q>, glm::vec<N, T, Q>>,
      public gal::view::Slider<glm::vec<N, T, Q>>
{
  using VecType = glm::vec<N, T, Q>;

public:
  SliderFunc(const std::string& label, const T& min, const T& max, const T& value)
      : gal::func::TVariable<VecType, VecType>(VecType(std::clamp(value, min, max)))
      , gal::view::Slider<VecType>(label, min, max, value) {};

private:
  using gal::view::Slider<VecType>::addHandler;

protected:
  void handleChanges() override
  {
    if (this->isEdited())
      this->set(this->mValue);

    this->clearEdited();
  };
};

/**
 * @brief The whole purpose of having this struct is to do partial template
 * specialization.
 * @tparam T
 */
template<typename T>
struct makeSlider
{
  static boost::python::tuple make(boost::python::object pylabel,
                                   boost::python::object pymin,
                                   boost::python::object pymax,
                                   boost::python::object pyvalue)
  {
    std::string label;
    T           min, max, value;
    gal::func::Converter<decltype(pylabel), std::string>::assign(pylabel, label);
    gal::func::Converter<decltype(pymin), T>::assign(pymin, min);
    gal::func::Converter<decltype(pymax), T>::assign(pymax, max);
    gal::func::Converter<decltype(pyvalue), T>::assign(pyvalue, value);

    auto fn = gal::func::store::makeFunction<SliderFunc<T>>(label, min, max, value);
    inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
    return gal::func::pythonRegisterTuple(gal::func::types::makeOutputTuple<1>(*fn));
  }
};

/**
 * @brief This specialization is needed for glm vectors because the constructors of the
 * corresponding slider widgets expect args of type T for min, max and value.
 * For example for glm::vec3 the arguments would not be glm::vec3 (as suggested by the
 * original template), instad they are float.
 * @tparam N The length of the vector
 * @tparam T The type of the vector.
 * @tparam Q precision.
 */
template<int N, typename T, glm::qualifier Q>
struct makeSlider<glm::vec<N, T, Q>>
{
  static boost::python::tuple make(boost::python::object pylabel,
                                   boost::python::object pymin,
                                   boost::python::object pymax,
                                   boost::python::object pyvalue)
  {
    std::string label;
    T           min, max, value;
    gal::func::Converter<decltype(pylabel), std::string>::assign(pylabel, label);
    gal::func::Converter<decltype(pymin), T>::assign(pymin, min);
    gal::func::Converter<decltype(pymax), T>::assign(pymax, max);
    gal::func::Converter<decltype(pyvalue), T>::assign(pyvalue, value);

    auto fn = gal::func::store::makeFunction<SliderFunc<glm::vec<N, T, Q>>>(
      label, min, max, value);
    inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
    return gal::func::pythonRegisterTuple(gal::func::types::makeOutputTuple<1>(*fn));
  }
};

template<typename T>
boost::python::tuple py_slider(boost::python::object pylabel,
                               boost::python::object pymin,
                               boost::python::object pymax,
                               boost::python::object pyvalue)
{
  return makeSlider<T>::make(pylabel, pymin, pymax, pyvalue);
};

}  // namespace viewfunc
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();

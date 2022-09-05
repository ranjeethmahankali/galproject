#pragma once

#include <galview/Context.h>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>

#include <galfunc/Functions.h>
#include <galview/Interaction.h>

namespace gal {
namespace viewfunc {

view::Panel& inputsPanel();

void evalOutputs();

/**
 * @brief Clears all output registers.
 * This is meant to be called when unloading a demo.
 */
void unloadAllOutputs();

/**
 * @brief Function that wraps a gui slider that can be used to interactively control the
 * output value.
 *
 * @tparam T The output type of the slider.
 */
template<typename T>
struct SliderFunc : public gal::func::TVariable<T>, public gal::view::Slider<T>
{
public:
  using PyOutuptType = typename gal::func::TVariable<T>::PyOutputType;
  SliderFunc(const std::string& label, T min, T max, T value)
      : gal::func::TVariable<T>(value)
      , gal::view::Slider<T>(label, min, max, value) {};

  virtual ~SliderFunc() = default;

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

/**
 * @brief Partial specialization of the slider template for vectors.
 *
 * @tparam N The number of components in the vector.
 * @tparam T The datatype of the vector.
 * @tparam Q The precision.
 */
template<int N, typename T, glm::qualifier Q>
struct SliderFunc<glm::vec<N, T, Q>> : public gal::func::TVariable<glm::vec<N, T, Q>>,
                                       public gal::view::Slider<glm::vec<N, T, Q>>
{
  using VecType = glm::vec<N, T, Q>;

public:
  SliderFunc(const std::string& label, const T& min, const T& max, const T& value)
      : gal::func::TVariable<VecType>(VecType(std::clamp(value, min, max)))
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
  static typename SliderFunc<T>::PyOutuptType make(py::object pylabel,
                                                   py::object pymin,
                                                   py::object pymax,
                                                   py::object pyvalue)
  {
    std::string label;
    T           min, max, value;
    gal::func::Converter<decltype(pylabel), std::string>::assign(pylabel, label);
    gal::func::Converter<decltype(pymin), T>::assign(pymin, min);
    gal::func::Converter<decltype(pymax), T>::assign(pymax, max);
    gal::func::Converter<decltype(pyvalue), T>::assign(pyvalue, value);

    static const std::string      sName = "slider_" + TypeInfo<T>::name();
    static const std::string      sDesc = "Slider for type " + TypeInfo<T>::name() + ".";
    static const std::string_view sOutputName = "value";
    static const std::string_view sOutputDesc = "Value output from the slider";

    static const func::FuncInfo sInfo = {{sName.data(), sName.size()},
                                         {sDesc.data(), sDesc.size()},
                                         0,
                                         nullptr,
                                         nullptr,
                                         1,
                                         &sOutputName,
                                         &sOutputDesc};

    auto fn =
      gal::func::store::makeFunction<SliderFunc<T>>(sInfo, label, min, max, value);
    inputsPanel().addWidget(dynamic_cast<gal::view::Widget*>(fn));
    return dynamic_cast<const SliderFunc<T>*>(fn)->pythonOutputRegs();
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
  static typename SliderFunc<glm::vec<N, T, Q>>::PyOutputType make(py::object pylabel,
                                                                   py::object pymin,
                                                                   py::object pymax,
                                                                   py::object pyvalue)
  {
    std::string label;
    T           min, max, value;
    gal::func::Converter<decltype(pylabel), std::string>::assign(pylabel, label);
    gal::func::Converter<decltype(pymin), T>::assign(pymin, min);
    gal::func::Converter<decltype(pymax), T>::assign(pymax, max);
    gal::func::Converter<decltype(pyvalue), T>::assign(pyvalue, value);

    static const std::string sName = "slider_" + TypeInfo<glm::vec<N, T, Q>>::name();
    static const std::string sDesc =
      "Slider for type " + TypeInfo<glm::vec<N, T, Q>>::name() + ".";
    static const std::string_view sOutputName = "value";
    static const std::string_view sOutputDesc = "Value output from the slider";

    static const func::FuncInfo sInfo = {{sName.data(), sName.size()},
                                         {sDesc.data(), sDesc.size()},
                                         0,
                                         nullptr,
                                         nullptr,
                                         1,
                                         &sOutputName,
                                         &sOutputDesc};

    auto fn = gal::func::store::makeFunction<SliderFunc<glm::vec<N, T, Q>>>(
      sInfo, label, min, max, value);
    inputsPanel().addWidget(dynamic_cast<gal::view::Widget*>(fn));
    return dynamic_cast<const SliderFunc<glm::vec<N, T, Q>>*>(fn)->pythonOutputRegs();
  }
};

template<typename T>
typename SliderFunc<T>::PyOutputType py_slider(py::object pylabel,
                                               py::object pymin,
                                               py::object pymax,
                                               py::object pyvalue)
{
  return makeSlider<T>::make(pylabel, pymin, pymax, pyvalue);
};

}  // namespace viewfunc
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();

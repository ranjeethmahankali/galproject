#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <galcore/Annotations.h>
#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeHelper.h>
#include <galview/AnnotationsView.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>
#include <galview/Views.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

static view::Panel*                                           sInputPanel  = nullptr;
static view::Panel*                                           sOutputPanel = nullptr;
static std::vector<std::function<void()>>                     sOutputCallbacks;
static std::unordered_map<std::string, const view::CheckBox&> sShowCheckboxes;

/**
 * @brief Gets the visibility checkbox from the output panel with the given name.
 * If a checkbox with the given name doesn't exist, one will be added.
 * @param name the unique name that identifies the checkbox.
 */
static const view::CheckBox& getCheckBox(const std::string name)
{
  auto match = sShowCheckboxes.find(name);
  if (match != sShowCheckboxes.end()) {
    return match->second;
  }
  auto pair =
    sShowCheckboxes.emplace(name, *(outputPanel().newWidget<view::CheckBox>(name, true)));
  return pair.first->second;
}

void initPanels(view::Panel& inputs, view::Panel& outputs)
{
  sInputPanel  = &inputs;
  sOutputPanel = &outputs;
}

view::Panel& inputPanel()
{
  return *sInputPanel;
}

view::Panel& outputPanel()
{
  return *sOutputPanel;
}

void evalOutputs()
{
  for (auto& cbfn : sOutputCallbacks) {
    cbfn();
  }
}

void unloadAllOutputs()
{
  std::cout << "Unloading all output data...\n";
  sOutputCallbacks.clear();
  sInputPanel->clear();
  sOutputPanel->clear();
}

boost::python::list py_loadGlyphs(const boost::python::list& pyglyphdata)
{
  std::vector<std::pair<std::string, fs::path>> glyphData;
  func::Converter<boost::python::list, decltype(glyphData)>::assign(pyglyphdata,
                                                                    glyphData);
  view::loadGlyphs(glyphData);
  boost::python::list indices;
  for (const auto& g : glyphData) {
    indices.append(view::getGlyphIndex(std::get<0>(g)));
  }

  return indices;
}

GAL_FUNC(
  glyphs,
  "Displays glyphs in the viewer",
  (((func::data::ReadView<int, 1>), indices, "The indices of the glyphs to be displayed"),
   ((func::data::ReadView<glm::vec3, 1>),
    positions,
    "Positions at which to display the glyphs")),
  ((gal::GlyphAnnotations, result, "The glyph set")))
{
  if (indices.size() != positions.size()) {
    throw std::length_error("Index list and positions list must have the same length");
  }
  result.resize(indices.size());
  for (size_t i = 0; i < indices.size(); i++) {
    result[i] = {positions[i], {uint32_t(indices[i])}};
  }
}

GAL_FUNC(
  tags,
  "Shows string tags in the viewer",
  (((func::data::ReadView<std::string, 1>), words, "The string tags to show."),
   ((func::data::ReadView<glm::vec3, 1>), positions, "Positions to show the tags at")),
  ((gal::TextAnnotations, result, "The tags")))
{
  if (words.size() != positions.size()) {
    throw std::length_error(
      "The number of tags must be the same as the number of positions.");
  }
  result.resize(words.size());
  for (size_t i = 0; i < words.size(); i++) {
    result[i] = {positions[i], words[i]};
  }
}

struct TextFieldFunc : public gal::func::TVariable<std::string>,
                       public gal::view::TextInput
{
public:
  using PyOutputType = typename gal::func::TVariable<std::string>::PyOutputType;
  TextFieldFunc(const std::string& label)
      : gal::func::TVariable<std::string>("")
      , gal::view::TextInput(label, "") {};

private:
  using gal::view::TextInput::addHandler;

protected:
  void handleChanges() override
  {
    if (isEdited())
      this->set(this->mValue);

    clearEdited();
  }
};

// Simple functions that are not part of the functional system.
void py_set2dMode(bool flag)
{
  gal::view::Context::get().set2dMode(flag);
};

void py_useOrthoCam()
{
  gal::view::Context::get().setOrthographic();
}

void py_usePerspectiveCam()
{
  gal::view::Context::get().setPerspective();
}

typename TextFieldFunc::PyOutputType py_textField(const std::string& label)
{
  auto fn = gal::func::store::makeFunction<TextFieldFunc>(label);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return fn->pythonOutputRegs();
};

/**
 * @brief Function that adds the given object to the 3d scene, if it is a drawable object.
 * The output type of the function is the draw-id of the object added to the scene.
 *
 * @tparam T The object to be drawn.
 */
template<typename T>
struct ShowFunc : public func::TFunction<const func::data::Tree<T>, uint64_t>
{
  static_assert(TypeInfo<T>::value, "Unknown type");
  using BaseT        = func::TFunction<const func::data::Tree<T>, uint64_t>;
  using PyOutputType = typename BaseT::PyOutputType;

  ShowFunc(const std::string&       label,
           const bool*              visibilityFlag,
           const func::Register<T>& reg)
      : BaseT(
          [visibilityFlag](const func::data::Tree<T>& objs, uint64_t& id) {
            if constexpr (view::Drawable<T>::value) {
              id = view::Views::add<T>(objs.values(), visibilityFlag, id);
            }
            else {
              std::cerr << TypeInfo<T>::name() << " is not a drawable type\n";
            }
          },
          std::make_tuple(reg))
  {
    auto& tree = std::get<0>(this->mOutputs);
    tree.resize(1);
    tree.value(0) = 0;
  }
  virtual ~ShowFunc() = default;
};

/**
 * @brief
 *
 * @tparam T The type of object to be added to the scene.
 * @param label The name of the object. This will be the label of the checkbox that
 * controls the visibility of the object.
 *
 * @param reg The register that contains the object to be drawn.
 * @return Register that contains the draw-id of the object added to the scene.
 */
template<typename T>
typename ShowFunc<T>::PyOutputType py_show(const std::string&       label,
                                           const func::Register<T>& reg)
{
  std::shared_ptr<ShowFunc<T>> sfn = gal::func::store::makeFunction<ShowFunc<T>>(
    label, getCheckBox(label).checkedPtr(), reg);
  auto fn = std::dynamic_pointer_cast<func::Function>(sfn);
  func::store::addFunction(fn);
  const func::Function* ptr = fn.get();
  sOutputCallbacks.push_back([ptr]() { ptr->update(); });
  return sfn->pythonOutputRegs();
}

/**
 * @brief Prints the given object to the output panel.
 *
 * @tparam T The type of the object to be printed.
 */
template<typename T>
struct PrintFunc : public func::TFunction<const func::data::Tree<T>, uint8_t>,
                   public view::Text
{
  using BaseT        = func::TFunction<const func::data::Tree<T>, uint8_t>;
  using PyOutputType = typename BaseT::PyOutputType;

  std::string       mLabel;
  std::stringstream mStream;

  PrintFunc(const std::string& label, const func::Register<T>& reg)
      : mLabel(label)
      , view::Text("")
      , BaseT(
          [this](const func::data::Tree<T>& obj, uint8_t& success) {
            mStream.clear();
            mStream.str("");
            mStream << mLabel << ": \n" << obj;
            this->mValue = mStream.str();
            success      = 1;
          },
          std::make_tuple(reg)) {};

  virtual ~PrintFunc() = default;
};

/**
 * @brief Prints the object to the output panel.
 *
 * @tparam T The type of the object to be printed.
 * @param label The label to be used in the output panel.
 * @param reg The register containing the object.
 * @return A boolean register indicating success status.
 */
template<typename T>
typename PrintFunc<T>::PyOutputType py_print(const std::string&       label,
                                             const func::Register<T>& reg)
{
  std::shared_ptr<PrintFunc<T>> pfn =
    gal::func::store::makeFunction<PrintFunc<T>>(label, reg);
  auto fn = std::dynamic_pointer_cast<func::Function>(pfn);
  func::store::addFunction(fn);
  const func::Function* ptr = fn.get();
  sOutputCallbacks.push_back([ptr]() { ptr->update(); });
  outputPanel().addWidget(std::dynamic_pointer_cast<view::Widget>(pfn));
  return pfn->pythonOutputRegs();
}

namespace python {
using namespace boost::python;

template<typename T>
struct defOutputFuncs
{
  static void invoke()
  {
    def("show", py_show<T>);
    def("print", py_print<T>);
  }
};

}  // namespace python

}  // namespace viewfunc
}  // namespace gal

#define GAL_DEF_PY_FN(fnName) def(#fnName, py_##fnName);

BOOST_PYTHON_MODULE(pygalview)
{
  using namespace boost::python;
  using namespace gal::viewfunc;
  using namespace gal::viewfunc::python;
  // Sliders for float input
  def("sliderf32", gal::viewfunc::py_slider<float>);
  def("slideri32", gal::viewfunc::py_slider<int32_t>);
  def("sliderVec3", gal::viewfunc::py_slider<glm::vec3>);
  def("sliderVec2", gal::viewfunc::py_slider<glm::vec2>);

  gal::func::typemanager::invoke<defOutputFuncs>();

  // Text fields for string inputs
  GAL_DEF_PY_FN(textField);
  // Viewer annotations
  GAL_DEF_PY_FN(tags);
  GAL_DEF_PY_FN(glyphs);
  GAL_DEF_PY_FN(loadGlyphs);
  // Viewer controls.
  GAL_DEF_PY_FN(set2dMode);
  GAL_DEF_PY_FN(useOrthoCam);
  GAL_DEF_PY_FN(usePerspectiveCam);
};

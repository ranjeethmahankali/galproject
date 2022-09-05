#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <Annotations.h>
#include <AnnotationsView.h>
#include <Command.h>
#include <Context.h>
#include <Data.h>
#include <Functions.h>
#include <GuiFunctions.h>
#include <Interaction.h>
#include <TypeManager.h>
#include <Types.h>
#include <Util.h>
#include <Views.h>
#include <pybind11/pybind11.h>

namespace gal {
namespace viewfunc {

static bool                               sShowInputs  = true;
static bool                               sShowOutputs = true;
static std::vector<const func::Function*> sOutputFuncs;
static std::unordered_map<std::string, std::unique_ptr<view::CheckBox>> sShowCheckboxes;

view::Panel& outputsPanel()
{
  return view::panelByName("outputs");
}

view::Panel& inputsPanel()
{
  return view::panelByName("inputs");
}

/**
 * @brief Gets the visibility checkbox from the output panel with the given name.
 * If a checkbox with the given name doesn't exist, one will be added.
 * @param name the unique name that identifies the checkbox.
 */
static const view::CheckBox& getCheckBox(const std::string name)
{
  auto match = sShowCheckboxes.find(name);
  if (match != sShowCheckboxes.end()) {
    return *(match->second);
  }
  auto pair = sShowCheckboxes.emplace(name, std::make_unique<view::CheckBox>(name, true));
  outputsPanel().addWidget(pair.first->second.get());
  return *(pair.first->second);
}

void evalOutputs()
{
  for (const func::Function* fnptr : sOutputFuncs) {
    fnptr->update();
  }
}

void unloadAllOutputs()
{
  gal::view::logger().debug("Unloading all output data...");
  sOutputFuncs.clear();
  sShowCheckboxes.clear();
  inputsPanel().clear();
  outputsPanel().clear();
}

/**
 * @brief Loads glyph textures from files on disk.
 *
 * @param pyglyphdata Paths to png images.
 * @return py::list List of indices of the loaded glyphs, in the same order of
 * the supplied paths.
 */
py::list py_loadGlyphs(const py::list& glyphPaths)
{
  std::vector<fs::path> glyphData;
  func::Converter<py::list, decltype(glyphData)>::assign(glyphPaths, glyphData);
  auto     indices = view::loadGlyphs(glyphData);
  py::list lst;
  func::Converter<decltype(indices), py::list>::assign(indices, lst);
  return lst;
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
  static const std::string_view sOutputName = "text";
  static const std::string_view sOutputDesc = "The text from the text field input.";

  static const func::FuncInfo sFnInfo = {"text",
                                         "Text field input widget",
                                         0,
                                         nullptr,
                                         nullptr,
                                         1,
                                         &sOutputName,
                                         &sOutputDesc};
  auto fn = gal::func::store::makeFunction<TextFieldFunc>(sFnInfo, label);
  inputsPanel().addWidget(dynamic_cast<gal::view::Widget*>(fn));
  return dynamic_cast<const TextFieldFunc*>(fn)->pythonOutputRegs();
};

template<typename T>
struct ShowCallable
{
  static_assert(view::Views::IsDrawableType<T>, "Must be a drawable type");

  size_t mDrawableIndex;

  ShowCallable(const bool* visibilityFlag)
      : mDrawableIndex(view::Views::create<T>(visibilityFlag))
  {}

  void operator()(const func::data::Tree<T>& objs) const
  {
    view::Views::update<T>(mDrawableIndex, objs.values());
  }
};

/**
 * @brief Function that adds the given object to the 3d scene, if it is a drawable object.
 * The output type of the function is the draw-id of the object added to the scene.
 *
 * @tparam T The object to be drawn.
 */
template<typename T>
struct ShowFunc : public func::TFunction<ShowCallable<T>, const func::data::Tree<T>>
{
  static_assert(TypeInfo<T>::value, "Unknown type");
  using BaseT        = func::TFunction<ShowCallable<T>, const func::data::Tree<T>>;
  using PyOutputType = typename BaseT::PyOutputType;

  uint64_t mDrawId = 0;

  ShowFunc(const std::string&       label,
           const bool*              visibilityFlag,
           const func::Register<T>& reg)
      : BaseT(ShowCallable<T>(visibilityFlag), std::make_tuple(reg))
  {}

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
  static const std::string sName = "show_" + TypeInfo<T>::name();
  static const std::string sDesc = "Shows an object of type " + TypeInfo<T>::name() +
                                   " in the viewer, using the given label as the key.";
  static const std::string_view sInputName = "obj";
  static const std::string_view sInputDesc = "Object to be shown in the viewer.";

  static const func::FuncInfo sInfo = {{sName.data(), sName.size()},
                                       {sDesc.data(), sDesc.size()},
                                       1,
                                       &sInputName,
                                       &sInputDesc,
                                       0,
                                       nullptr,
                                       nullptr};

  static_assert(view::Views::IsDrawableType<T>);
  auto fn = gal::func::store::makeFunction<ShowFunc<T>>(
    sInfo, label, getCheckBox(label).checkedPtr(), reg);
  sOutputFuncs.push_back(fn);
  return dynamic_cast<ShowFunc<T>*>(fn)->pythonOutputRegs();
}

template<typename T>
struct PrintCallable
{
  std::string mLabel;
  // mutable std::stringstream mStream;
  gal::view::Text* mTextPtr = nullptr;

  PrintCallable(const std::string& label, gal::view::Text* textLabelPtr)
      : mLabel(label)
      , mTextPtr(textLabelPtr)
  {}

  void operator()(const func::data::Tree<T>& obj) const
  {
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << mLabel << ": \n" << obj;
    mTextPtr->value() = stream.str();
  }
};

/**
 * @brief Prints the given object to the output panel.
 *
 * @tparam T The type of the object to be printed.
 */
template<typename T>
struct PrintFunc : public func::TFunction<PrintCallable<T>, const func::data::Tree<T>>,
                   public view::Text
{
  using BaseT        = func::TFunction<PrintCallable<T>, const func::data::Tree<T>>;
  using PyOutputType = typename BaseT::PyOutputType;

  PrintFunc(const std::string& label, const func::Register<T>& reg)
      : view::Text("")
      , BaseT(PrintCallable<T>(label, this), std::make_tuple(reg)) {};

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
  static const std::string sName = "print_" + TypeInfo<T>::name();
  static const std::string sDesc =
    "Prints an object of type " + TypeInfo<T>::name() +
    " in the outputs panel, using the given label as the key.";
  static const std::string_view sInputName = "obj";
  static const std::string_view sInputDesc = "Object to be printed to the output panel.";

  static const func::FuncInfo sInfo = {{sName.data(), sName.size()},
                                       {sDesc.data(), sDesc.size()},
                                       1,
                                       &sInputName,
                                       &sInputDesc,
                                       0,
                                       nullptr,
                                       nullptr};

  auto fn = gal::func::store::makeFunction<PrintFunc<T>>(sInfo, label, reg);
  sOutputFuncs.push_back(fn);
  outputsPanel().addWidget(dynamic_cast<view::Widget*>(fn));
  return dynamic_cast<PrintFunc<T>*>(fn)->pythonOutputRegs();
}

void py_runCommands(const std::string& commands)
{
  view::queueCommands(commands);
}

namespace python {

template<typename T>
struct defOutputFuncs
{
  static void invoke(py::module& mod)
  {
    mod.def(
      "print",
      py_print<T>,
      "Prints the given object (second arg) to the output panel with the given label "
      "(first arg)");
    if constexpr (view::Views::IsDrawableType<T>) {
      mod.def("show",
              py_show<T>,
              "Shows the given object (second arg) in the viewer, with checkbox with the "
              "given label (first arg) to control the visibility.");
    }
  }
};

}  // namespace python

}  // namespace viewfunc
}  // namespace gal

#define GAL_DEF_PY_FN(fnName, mod) mod.def(#fnName, py_##fnName);
#define GAL_DEF_PY_FN_DOC(fnName, mod, docstr) mod.def(#fnName, py_##fnName, docstr);

PYBIND11_MODULE(pygalview, pgv)
{
  using namespace gal::viewfunc;
  using namespace gal::viewfunc::python;
  // Sliders for float input
  pgv.def(
    "sliderf32",
    gal::viewfunc::py_slider<float>,
    "Float-32 slider with the given label, min value, max value and initial value.");
  pgv.def("slideri32",
          gal::viewfunc::py_slider<int32_t>,
          "Int-32 slider with the given label, min value, max value and initial value.");
  pgv.def("sliderVec3",
          gal::viewfunc::py_slider<glm::vec3>,
          "Vec3 slider with the given label, min value, max value and initial value.");
  pgv.def("sliderVec2",
          gal::viewfunc::py_slider<glm::vec2>,
          "Vec2 slider with the given label, min value, max value and initial value.");

  gal::func::typemanager::invoke<defOutputFuncs>((py::module&)pgv);

  // Text fields for string inputs
  GAL_DEF_PY_FN_DOC(textField, pgv, "Creates a text field with the given label.");
  // Viewer annotations
  GAL_DEF_PY_FN(tags, pgv);
  GAL_DEF_PY_FN(glyphs, pgv);
  GAL_DEF_PY_FN(loadGlyphs, pgv);
  // Allows demos to run viewer commands.
  GAL_DEF_PY_FN(runCommands, pgv);
};

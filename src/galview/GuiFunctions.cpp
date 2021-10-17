#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/GeomFunctions.h>
#include <galfunc/MeshFunctions.h>
#include <galfunc/TypeHelper.h>
#include <galview/AllViews.h>
#include <galview/AnnotationsView.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>
#include <galview/Widget.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>

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

// TODO: TagsFunc
// TODO: GlyphsFunc

struct TextFieldFunc : public gal::func::TVariable<std::string, std::string>,
                       public gal::view::TextInput
{
public:
  using PyOutputType =
    typename gal::func::TVariable<std::string, std::string>::PyOutputType;
  TextFieldFunc(const std::string& label)
      : gal::func::TVariable<std::string, std::string>("")
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

// TODO: py_tags
// TODO: py_glyphs

void py_loadGlyphs(const boost::python::list& lst)
{
  std::vector<std::pair<std::string, std::string>> pairs;
  gal::func::Converter<boost::python::list, decltype(pairs)>::assign(lst, pairs);

  std::vector<std::pair<std::string, fs::path>> pairs2(pairs.size());
  std::transform(pairs.begin(),
                 pairs.end(),
                 pairs2.begin(),
                 [](const std::pair<std::string, std::string>& p) {
                   return std::make_pair(p.first, fs::path(p.second));
                 });
  gal::view::loadGlyphs(pairs2);
}

int32_t py_glyphIndex(const std::string& str)
{
  return int32_t(gal::view::getGlyphIndex(str));
}

/**
 * @brief Function that adds the given object to the 3d scene, if it is a drawable object.
 * The output type of the function is the draw-id of the object added to the scene.
 *
 * @tparam T The object to be drawn.
 */
template<typename T>
struct ShowFunc : public func::TFunction<const T, uint64_t>
{
  static_assert(TypeInfo<T>::value, "Unknown type");
  using PyOutputType = typename func::TFunction<const T, uint64_t>::PyOutputType;

  ShowFunc(const std::string&       label,
           const bool*              visibilityFlag,
           const func::Register<T>& reg)
      : func::TFunction<const T, uint64_t>(
          [this, visibilityFlag](const T& obj, uint64_t& id) {
            if constexpr (view::MakeDrawable<T>::value) {
              id = view::Context::get().addDrawable(obj, visibilityFlag, id);
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
struct PrintFunc : public func::TFunction<const T, uint8_t>, public view::Text
{
  using PyOutputType = typename func::TFunction<const T, uint8_t>::PyOutputType;
  std::string       mLabel;
  std::stringstream mStream;

  PrintFunc(const std::string& label, const func::Register<T>& reg)
      : mLabel(label)
      , view::Text("")
      , func::TFunction<const T, uint8_t>(
          [this](const T& obj, uint8_t& success) {
            if constexpr (IsPrintable<T>::value) {
              mStream.clear();
              mStream.str("");
              mStream << mLabel << ": " << obj;
              this->mValue = mStream.str();
              success      = 1;
            }
            else {
              std::cerr << TypeInfo<T>::name() << " object";
            }
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
struct defOutputFuncs : public gal::func::python::defClass<T>
{
  using BaseType = gal::func::python::defClass<T>;

  static void invoke()
  {
    def("show", py_show<T>);
    def("print", py_print<T>);
  }
};

}  // namespace python

}  // namespace viewfunc
}  // namespace gal

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

  // Views for drawables
  // GAL_DEF_PY_FN(show);
  // GAL_DEF_PY_FN(showAll);
  // Labels for printable data
  // GAL_DEF_PY_FN(print);
  // Text fields for string inputs
  GAL_DEF_PY_FN(textField);
  // Viewer annotations
  // GAL_DEF_PY_FN(tags);
  // GAL_DEF_PY_FN(glyphs);
  GAL_DEF_PY_FN(loadGlyphs);
  GAL_DEF_PY_FN(glyphIndex);
  // Viewer controls.
  GAL_DEF_PY_FN(set2dMode);
  GAL_DEF_PY_FN(useOrthoCam);
  GAL_DEF_PY_FN(usePerspectiveCam);
};

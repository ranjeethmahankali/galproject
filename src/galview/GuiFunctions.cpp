#include <sstream>

#include <galfunc/GeomFunctions.h>
#include <galfunc/MeshFunctions.h>
#include <galview/AllViews.h>
#include <galview/AnnotationsView.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>

namespace gal {
namespace viewfunc {

static view::Panel*          sInputPanel  = nullptr;
static view::Panel*          sOutputPanel = nullptr;
static std::vector<uint64_t> sShowFuncRegs;
static std::vector<uint64_t> sPrintFuncRegs;

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
  for (uint64_t id : sShowFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
  for (uint64_t id : sPrintFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
}

void unloadAllOutputs()
{
  std::cout << "Unloading all output data...\n";
  sShowFuncRegs.clear();
  sPrintFuncRegs.clear();
  sInputPanel->clear();
  sOutputPanel->clear();
}

template<typename T, typename... TRest>
struct DrawableManager
{
  static size_t draw(uint64_t                     typeId,
                     const std::shared_ptr<void>& ptr,
                     size_t                       oldDrawId,
                     const bool*                  visibility)
  {
    static_assert(gal::TypeInfo<T>::value, "Unknown type");
    if (typeId == gal::TypeInfo<T>::id) {
      auto castsp = std::static_pointer_cast<T>(ptr);
      return view::Context::get().replaceDrawable<T>(oldDrawId, *castsp, visibility);
    }
    else if constexpr (sizeof...(TRest) > 0) {
      return DrawableManager<TRest...>::draw(typeId, ptr, oldDrawId, visibility);
    }
    else if constexpr (sizeof...(TRest) == 0) {
      std::cerr << "Datatype " << gal::TypeInfo<T>::name()
                << " is not a drawable object\n";
      throw std::bad_cast();
    }
  };
};
using dmanager = DrawableManager<gal::Box3,
                                 gal::PointCloud,
                                 gal::Sphere,
                                 gal::Circle2d,
                                 gal::Mesh,
                                 gal::Plane,
                                 gal::Line2d,
                                 gal::Line3d>;

struct ShowFunc : public gal::func::DynamicFunction, public gal::view::CheckBox
{
  ShowFunc(const std::string& label, uint64_t regId)
      : mSuccess(std::make_shared<bool>(false))
      , mDrawIds(1, 0)
      , gal::func::DynamicFunction(std::vector<uint64_t> {regId}, 1)
      , gal::view::CheckBox(label, true)
  {}

  ShowFunc(const std::string& label, const std::vector<uint64_t>& regIds)
      : mSuccess(std::make_shared<bool>(false))
      , mDrawIds(regIds.size(), 0)
      , gal::func::DynamicFunction(regIds, 1)
      , gal::view::CheckBox(label, true)
  {}

  virtual ~ShowFunc() = default;

  void run() override
  {
    try {
      for (size_t i = 0; i < mInputs.size(); i++) {
        uint64_t& regId  = mInputs[i];
        uint64_t& drawId = mDrawIds[i];
        // Calling get triggers the upstream computations if needed.
        gal::func::store::get<void>(regId);
        auto& reg = gal::func::store::getRegister(regId);
        drawId    = dmanager::draw(reg.typeId, reg.ptr, drawId, checkedPtr());
      }
      *mSuccess = true;
    }
    catch (std::exception ex) {
      std::cerr << "Unable to render the object: " << ex.what() << std::endl;
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mOutputs[0], mSuccess);
  }

private:
  using gal::view::CheckBox::addHandler;

private:
  std::vector<size_t>   mDrawIds;
  std::shared_ptr<bool> mSuccess;
};

struct TagsFunc : public gal::func::DynamicFunction, public gal::view::CheckBox
{
  TagsFunc(const std::string& label, uint64_t locRegId, uint64_t wordsRegId)
      : mSuccess(std::make_shared<bool>(false))
      , gal::func::DynamicFunction({locRegId, wordsRegId}, 1)
      , gal::view::CheckBox(label, true)
  {}

  virtual ~TagsFunc() = default;

  void run() override
  {
    try {
      // Calling get triggers the upstream computations if needed.
      auto  locs     = gal::func::store::get<std::vector<glm::vec3>>(mInputs[0]);
      auto& locsReg  = gal::func::store::getRegister(mInputs[0]);
      auto  words    = gal::func::store::get<std::vector<std::string>>(mInputs[1]);
      auto& wordsReg = gal::func::store::getRegister(mInputs[1]);

      size_t          ntags = std::min(locs->size(), words->size());
      TextAnnotations tagvals;
      tagvals.reserve(ntags);
      for (size_t i = 0; i < ntags; i++) {
        tagvals.emplace_back(locs->at(i), words->at(i));
      }

      mDrawId = gal::view::Context::get().replaceDrawable(mDrawId, tagvals, checkedPtr());
      *mSuccess = true;
    }
    catch (std::exception ex) {
      std::cerr << "Unable to render text tags: " << ex.what() << std::endl;
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mOutputs[0], mSuccess);
  }

private:
  using gal::view::CheckBox::addHandler;

private:
  size_t                mDrawId = 0;
  std::shared_ptr<bool> mSuccess;
};

struct GlyphsFunc : public gal::func::DynamicFunction, public gal::view::CheckBox
{
  GlyphsFunc(const std::string& label, uint64_t locsRegId, uint64_t glyphsRegId)
      : mSuccess(std::make_shared<bool>(false))
      , gal::func::DynamicFunction({locsRegId, glyphsRegId}, 1)
      , gal::view::CheckBox(label, true)
  {}

  virtual ~GlyphsFunc() = default;

  void run() override
  {
    try {
      // Calling get triggers the upstream computations if needed.
      auto  locs      = gal::func::store::get<std::vector<glm::vec3>>(mInputs[0]);
      auto& locsReg   = gal::func::store::getRegister(mInputs[0]);
      auto  glyphs    = gal::func::store::get<std::vector<int32_t>>(mInputs[1]);
      auto& glyphsReg = gal::func::store::getRegister(mInputs[1]);

      size_t           ntags = std::min(locs->size(), glyphs->size());
      GlyphAnnotations tagvals;
      tagvals.reserve(ntags);
      for (size_t i = 0; i < ntags; i++) {
        tagvals.emplace_back(locs->at(i), gal::Glyph {uint32_t(glyphs->at(i))});
      }

      mDrawId = gal::view::Context::get().replaceDrawable(mDrawId, tagvals, checkedPtr());
      *mSuccess = true;
    }
    catch (std::exception ex) {
      std::cerr << "Unable to render glyphs: " << ex.what() << std::endl;
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mOutputs[0], mSuccess);
  }

private:
  size_t                mDrawId = 0;
  std::shared_ptr<bool> mSuccess;
};

template<typename T, typename... TRest>
struct PrintManager
{
  static std::string print(uint64_t typeId, const std::shared_ptr<void>& ptr)
  {
    static_assert(gal::TypeInfo<T>::value, "Unknown type");
    if (typeId == gal::TypeInfo<T>::id) {
      auto              castsp = std::static_pointer_cast<T>(ptr);
      std::stringstream ss;
      ss << *castsp;
      return ss.str();
    }
    else if (typeId == gal::TypeInfo<std::vector<T>>::id) {
      auto              castsp = std::static_pointer_cast<std::vector<T>>(ptr);
      std::stringstream ss;
      ss << *castsp;
      return ss.str();
    }
    else if constexpr (sizeof...(TRest) > 0) {
      return PrintManager<TRest...>::print(typeId, ptr);
    }
    else if constexpr (sizeof...(TRest) == 0) {
      std::cerr << "Datatype " << gal::TypeInfo<T>::name()
                << " is not a printable object\n";
      throw std::bad_cast();
    }
  };
};

using printmanager = PrintManager<float, int32_t, glm::vec3, glm::vec2, std::string>;

struct PrintFunc : public gal::func::DynamicFunction, public gal::view::Text
{
private:
  std::string           mLabel;
  std::shared_ptr<bool> mSuccess;

public:
  PrintFunc(const std::string& label, uint64_t regId)
      : mSuccess(std::make_shared<bool>(false))
      , mLabel(label)
      , gal::func::DynamicFunction(std::vector<uint64_t> {regId}, 1)
      , gal::view::Text("") {};

  virtual ~PrintFunc() = default;

  void run() override
  {
    try {
      auto  obj    = gal::func::store::get<void>(mInputs[0]);
      auto& reg    = gal::func::store::getRegister(mInputs[0]);
      this->mValue = mLabel + ": " + printmanager::print(reg.typeId, reg.ptr);
      *mSuccess    = true;
    }
    catch (std::exception ex) {
      std::cerr << "Unable to print the data: " << ex.what() << std::endl;
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mOutputs[0], mSuccess);
  };
};

struct TextFieldFunc : public gal::func::TVariable<std::string, std::string>,
                       public gal::view::TextInput
{
public:
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

gal::func::PyFnOutputType<1> py_show(const std::string&         label,
                                     gal::func::store::Register reg)
{
  using namespace gal::func;
  auto fn = store::makeFunction<ShowFunc>(label, reg.id);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
};

gal::func::PyFnOutputType<1> py_showAll(const std::string&         label,
                                        const boost::python::list& regsPy)
{
  using namespace gal::func;
  std::vector<gal::func::store::Register> regs;
  gal::func::Converter<boost::python::list, decltype(regs)>::assign(regsPy, regs);
  std::vector<uint64_t> regIds(regs.size());
  std::transform(
    regs.begin(), regs.end(), regIds.begin(), [](const gal::func::store::Register& reg) {
      return reg.id;
    });
  auto fn = store::makeFunction<ShowFunc>(label, regIds);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
};

gal::func::PyFnOutputType<1> py_print(const std::string&         label,
                                      gal::func::store::Register reg)
{
  using namespace gal::func;
  auto fn = store::makeFunction<PrintFunc>(label, reg.id);
  sPrintFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
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

gal::func::PyFnOutputType<1> py_textField(const std::string& label)
{
  auto fn = gal::func::store::makeFunction<TextFieldFunc>(label);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(gal::func::types::makeOutputTuple<1>(*fn));
};

gal::func::PyFnOutputType<1> py_tags(const std::string&         label,
                                     gal::func::store::Register locs,
                                     gal::func::store::Register words)
{
  using namespace gal::func;
  auto fn = store::makeFunction<TagsFunc>(label, locs.id, words.id);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
}

gal::func::PyFnOutputType<1> py_glyphs(const std::string&         label,
                                       gal::func::store::Register locs,
                                       gal::func::store::Register glyphs)
{
  using namespace gal::func;
  auto fn = store::makeFunction<GlyphsFunc>(label, locs.id, glyphs.id);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
}

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

}  // namespace viewfunc
}  // namespace gal

BOOST_PYTHON_MODULE(pygalview)
{
  using namespace boost::python;
  using namespace gal::viewfunc;
  // Sliders for float input
  def("sliderf32", gal::viewfunc::py_slider<float>);
  def("slideri32", gal::viewfunc::py_slider<int32_t>);
  def("sliderVec3", gal::viewfunc::py_slider<glm::vec3>);
  def("sliderVec2", gal::viewfunc::py_slider<glm::vec2>);
  // Views for drawables
  GAL_DEF_PY_FN(show);
  GAL_DEF_PY_FN(showAll);
  // Labels for printable data
  GAL_DEF_PY_FN(print);
  // Text fields for string inputs
  GAL_DEF_PY_FN(textField);
  // Viewer annotations
  GAL_DEF_PY_FN(tags);
  GAL_DEF_PY_FN(glyphs);
  GAL_DEF_PY_FN(loadGlyphs);
  GAL_DEF_PY_FN(glyphIndex);
  // Viewer controls.
  GAL_DEF_PY_FN(set2dMode);
  GAL_DEF_PY_FN(useOrthoCam);
  GAL_DEF_PY_FN(usePerspectiveCam);
};

#include <galfunc/GeomFunctions.h>
#include <galfunc/MeshFunctions.h>
#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>
#include <sstream>

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
};

view::Panel& inputPanel()
{
  return *sInputPanel;
};

view::Panel& outputPanel()
{
  return *sOutputPanel;
};

void evalOutputs()
{
  for (uint64_t id : sShowFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
  for (uint64_t id : sPrintFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
};

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

struct ShowFunc : public gal::func::Function, public gal::view::CheckBox
{
  ShowFunc(const std::string& label, uint64_t regId)
      : mShowables(1, std::make_pair(regId, 0))
      , mSuccess(std::make_shared<bool>(false))
      , gal::view::CheckBox(label, true)
  {
    useUpstreamRegisters();
  }

  ShowFunc(const std::string& label, const std::vector<uint64_t>& regIds)
      : mShowables(regIds.size())
      , mSuccess(std::make_shared<bool>(false))
      , gal::view::CheckBox(label, true)
  {
    std::transform(regIds.begin(), regIds.end(), mShowables.begin(), [](uint64_t reg) {
      return std::make_pair(reg, size_t(0));
    });
    useUpstreamRegisters();
  }

  void run() override
  {
    try {
      for (auto& showable : mShowables) {
        // Calling get triggers the upstream computations if needed.
        gal::func::store::get<void>(showable.first);
        auto& reg = gal::func::store::getRegister(showable.first);
        showable.second =
          dmanager::draw(reg.typeId, reg.ptr, showable.second, checkedPtr());
      }
      *mSuccess = true;
    }
    catch (std::bad_alloc ex) {
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mRegisterId, mSuccess);
  }

  void initOutputRegisters() override
  {
    mRegisterId = gal::func::store::allocate(
      this, gal::TypeInfo<bool>::id, gal::TypeInfo<bool>::name());
  }

  size_t   numInputs() const override { return mShowables.size(); }
  uint64_t inputRegister(size_t index) const override
  {
    if (index < numInputs()) {
      return mShowables[index].first;
    }
    throw std::out_of_range("Index out of range");
  }
  size_t   numOutputs() const override { return 1; }
  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
  }

private:
  using gal::view::CheckBox::addHandler;

  /* The first is the reg-id of the object being shown. The second is the drawId of that
   * same object that is tracked by the viewer. Knowing the reg-id is needed in order to
   * get retriggered when the upstream changes, and in order to get the latest geometry.
   * Knowing the drawId is needed in order to interact with the viewer.*/
  using Showable = std::pair<uint64_t, size_t>;

  void useUpstreamRegisters()
  {
    for (size_t i = 0; i < mShowables.size(); i++) {
      // Telling galfunc that this functor depends on this input register.
      gal::func::store::useRegister(this, mShowables[i].first);
    }
  }

private:
  std::vector<Showable> mShowables;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;  // Output
};

struct TagsFunc : public gal::func::Function, public gal::view::CheckBox
{
  TagsFunc(const std::string& label, uint64_t locRegId, uint64_t wordsRegId)
      : mLocsRegId(locRegId)
      , mWordsRegId(wordsRegId)
      , mSuccess(std::make_shared<bool>(false))
      , gal::view::CheckBox(label, true)
  {
    gal::func::store::useRegister(this, mLocsRegId);
    gal::func::store::useRegister(this, mWordsRegId);
  }

  void run() override
  {
    try {
      // Calling get triggers the upstream computations if needed.
      gal::func::store::get<std::vector<glm::vec3>>(mLocsRegId);
      gal::func::store::get<std::vector<std::string>>(mWordsRegId);
      auto& locsReg  = gal::func::store::getRegister(mLocsRegId);
      auto& wordsReg = gal::func::store::getRegister(mWordsRegId);
      auto  locs     = std::static_pointer_cast<std::vector<glm::vec3>>(locsReg.ptr);
      auto  words    = std::static_pointer_cast<std::vector<std::string>>(wordsReg.ptr);

      size_t      ntags = std::min(locs->size(), words->size());
      Annotations tagvals;
      tagvals.reserve(ntags);
      for (size_t i = 0; i < ntags; i++) {
        tagvals.emplace_back(locs->at(i), words->at(i));
      }

      mDrawId = gal::view::Context::get().replaceDrawable(mDrawId, tagvals, checkedPtr());
      *mSuccess = true;
    }
    catch (std::bad_alloc ex) {
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mRegisterId, mSuccess);
  }

  void initOutputRegisters() override
  {
    mRegisterId = gal::func::store::allocate(
      this, gal::TypeInfo<bool>::id, gal::TypeInfo<bool>::name());
  }
  size_t   numInputs() const override { return 2; }
  uint64_t inputRegister(size_t index) const override
  {
    if (index == 0) {
      return mLocsRegId;
    }
    else if (index == 1) {
      return mWordsRegId;
    }
    throw std::out_of_range("Index out of range");
  }
  size_t   numOutputs() const override { return 1; }
  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
  }

private:
  using gal::view::CheckBox::addHandler;

private:
  uint64_t              mLocsRegId;
  uint64_t              mWordsRegId;
  size_t                mDrawId = 0;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;
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

struct PrintFunc : public gal::func::Function, public gal::view::Text
{
private:
  std::string           mLabel;
  uint64_t              mObjRegId;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;

public:
  PrintFunc(const std::string& label, uint64_t regId)
      : gal::view::Text("")
      , mLabel(label)
      , mObjRegId(regId)
      , mSuccess(std::make_shared<bool>(false))
  {
    gal::func::store::useRegister(this, mObjRegId);
  };

  void run() override
  {
    try {
      auto  obj    = gal::func::store::get<void>(mObjRegId);
      auto& reg    = gal::func::store::getRegister(mObjRegId);
      this->mValue = mLabel + ": " + printmanager::print(reg.typeId, reg.ptr);
      *mSuccess    = true;
    }
    catch (std::bad_alloc ex) {
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mRegisterId, mSuccess);
  };
  void initOutputRegisters() override
  {
    mRegisterId = gal::func::store::allocate(
      this, gal::TypeInfo<bool>::id, gal::TypeInfo<bool>::name());
  }
  size_t   numInputs() const override { return 1; }
  uint64_t inputRegister(size_t index) const override
  {
    if (index == 0) {
      return mObjRegId;
    }
    throw std::out_of_range("Index out of range");
  }
  size_t   numOutputs() const override { return 1; };
  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
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

boost::python::tuple py_show(const std::string& label, gal::func::store::Register reg)
{
  using namespace gal::func;
  auto fn = store::makeFunction<ShowFunc>(label, reg.id);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
};

boost::python::tuple py_showAll(const std::string&         label,
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

boost::python::tuple py_print(const std::string& label, gal::func::store::Register reg)
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

boost::python::tuple py_textField(const std::string& label)
{
  auto fn = gal::func::store::makeFunction<TextFieldFunc>(label);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(gal::func::types::makeOutputTuple<1>(*fn));
};

boost::python::tuple py_tags(const std::string&         label,
                             gal::func::store::Register locs,
                             gal::func::store::Register words)
{
  using namespace gal::func;
  auto fn = store::makeFunction<TagsFunc>(label, locs.id, words.id);
  sShowFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
}

}  // namespace viewfunc
}  // namespace gal

BOOST_PYTHON_MODULE(pygalview)
{
  using namespace boost::python;
  using namespace gal::viewfunc;
  //   class_<gal::func::store::Register>("Register").def(self_ns::str(self_ns::self));
  // Bindings related to gui
  // Views for drawables
  // Labels for strings
  // Sliders for float input
  // Text fields for string inputs
  def("sliderf32", gal::viewfunc::py_slider<float>);
  def("slideri32", gal::viewfunc::py_slider<int32_t>);
  def("sliderVec3", gal::viewfunc::py_slider<glm::vec3>);
  def("sliderVec2", gal::viewfunc::py_slider<glm::vec2>);
  GAL_DEF_PY_FN(show);
  GAL_DEF_PY_FN(showAll);
  GAL_DEF_PY_FN(print);
  GAL_DEF_PY_FN(textField);
  GAL_DEF_PY_FN(tags);
  GAL_DEF_PY_FN(set2dMode);
  GAL_DEF_PY_FN(useOrthoCam);
  GAL_DEF_PY_FN(usePerspectiveCam);
};
